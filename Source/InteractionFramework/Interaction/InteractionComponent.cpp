
#include "InteractionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Interactable.h"
#include "Debug/InteractionDebugHelper.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CachedQueryResult = FInteractionQueryResult{};
	CachedQueryResult.bShouldShowPrompt = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	DebugHelper = NewObject<UInteractionDebugHelper>(this);
	DebugHelper->Initialize(this);
	DebugHelper->SetDrawTrace(bDebugDrawTrace);
	DebugHelper->SetDrawDuration(DebugDrawDuration);
	DebugHelper->SetEnabled(bDebugOverlayEnabled);
	
	InteractorActor = GetOwner();
	
	StartFocusScan();
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DebugHelper)
	{
		DebugHelper->SetEnabled(false);
	}

	StopFocusScan();
	ResetHold();
	ClearFocus();

	Super::EndPlay(EndPlayReason);
}

float UInteractionComponent::GetHoldProgress() const
{
	if (!bIsHolding || HoldDuration <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(HoldElapsed / HoldDuration, 0.f, 1.f);
}

void UInteractionComponent::StartFocusScan()
{
	if (!GetWorld()) return;

	GetWorld()->GetTimerManager().SetTimer(
		FocusScanTimer,
		this,
		&UInteractionComponent::PerformFocusScan,
		ScanInterval,
		true
	);
}

void UInteractionComponent::StopFocusScan()
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(FocusScanTimer);
}

void UInteractionComponent::PerformFocusScan()
{
	if (!bEnabled) return;
	
	AActor* NewActor = nullptr;
	TScriptInterface<IInteractable> NewInteractable;
	const bool bFound = FindInteractableInView(NewActor, NewInteractable);

	// Lost focus
	if (!bFound)
	{
		if (FocusedActor.IsValid())
		{
			ClearFocus();
		}
		return;
	}

	// Focus changed
	if (FocusedActor.Get() != NewActor)
	{
		SetFocused(NewActor, NewInteractable);
	}

	DebugPushSnapshot();
}

bool UInteractionComponent::GetViewPoint(FVector& OutViewLoc, FRotator& OutViewRot) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	AActor* PawnOwner = InteractorActor.Get();
	if (!IsValid(PawnOwner)) return false;

	// Prefer owner controller viewpoint (For player-controlled pawns).
	if (APawn* Pawn = Cast<APawn>(PawnOwner))
	{
		if (AController* Controller = Pawn->GetController())
		{
			Controller->GetPlayerViewPoint(OutViewLoc, OutViewRot);
			return true;
		}
	}

	if (const AActor* Owner = GetOwner())
	{
		Owner->GetActorEyesViewPoint(OutViewLoc, OutViewRot);
		return true;
	}

	return false;
}

bool UInteractionComponent::FindInteractableInView(AActor*& OutActor, TScriptInterface<IInteractable>& OutInteractable)

{
	OutActor = nullptr;
	OutInteractable = nullptr;

	UWorld* World = GetWorld();
	if (!World) return false;

	FVector ViewLoc;
	FRotator ViewRot;
	if (!GetViewPoint(ViewLoc, ViewRot)) return false;

	const FVector Start = ViewLoc;
	const FVector End = Start + (ViewRot.Vector() * TraceDistance);

	LastTraceStart = Start;
	LastTraceEnd = End;
	bLastTraceHit = false;
	LastHitActor = nullptr;
	LastHitResult = FHitResult();
	bLastHitWasInteractable = false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionTrace), false);
	if (bIgnoreOwner && InteractorActor.IsValid())
	{
		Params.AddIgnoredActor(InteractorActor.Get());
	}

	FHitResult Hit;
	bool bHit = false;

	if (TraceRadius <= 0.f)
	{
		bHit = World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params);
	}
	else
	{
		bHit = World->SweepSingleByChannel(
			Hit,
			Start,
			End,
			FQuat::Identity,
			TraceChannel,
			FCollisionShape::MakeSphere(TraceRadius),
			Params
		);
	}

	if (!bHit) return false;

	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor)) return false;

	if (!HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) return false;

	bLastTraceHit = true;
	LastHitActor = HitActor;
	LastHitResult = Hit;
	bLastHitWasInteractable = true;
	
	OutActor = HitActor;
	
	OutInteractable.SetObject(HitActor);
	OutInteractable.SetInterface(Cast<IInteractable>(HitActor));
	
	return true;
}

void UInteractionComponent::SetFocused(AActor* NewActor, const TScriptInterface<IInteractable> NewInteractable)
{
	ResetHold();

	AActor* Prev = FocusedActor.Get();

	if (IsValid(Prev) && Prev->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_OnFocusEnd(Prev, InteractorActor.Get());
	}
	
	FocusedActor = NewActor;
	FocusedInteractable = NewInteractable;

	if (IsValid(NewActor))
	{
		IInteractable::Execute_OnFocusStart(NewActor, InteractorActor.Get());
	}
	
	RefreshQuery();
	OnFocusChanged.Broadcast(NewActor, Prev);
}

void UInteractionComponent::ClearFocus()
{
	ResetHold();

	AActor* Prev = FocusedActor.Get();

	if (IsValid(Prev) && Prev->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_OnFocusEnd(Prev, InteractorActor.Get());
	}
	
	FocusedActor = nullptr;
	FocusedInteractable = nullptr;
	
	// Clear query for no prompt
	CachedQueryResult = FInteractionQueryResult{};
	CachedQueryResult.bShouldShowPrompt = false;

	OnFocusChanged.Broadcast(nullptr, Prev);
	OnQueryUpdated.Broadcast(CachedQueryResult);
}

void UInteractionComponent::RefreshQuery()
{
	if (!FocusedActor.IsValid())
	{
		CachedQueryResult = FInteractionQueryResult{};
		CachedQueryResult.bShouldShowPrompt = false;
		OnQueryUpdated.Broadcast(CachedQueryResult);
		DebugPushSnapshot();
		return;
	}

	AActor* Interactor = InteractorActor.Get();
	AActor* Target = FocusedActor.Get();

	if (!IsValid(Interactor) || !IsValid(Target))
	{
		ClearFocus();
		return;
	}

	CachedQueryResult = IInteractable::Execute_QueryInteraction(Target, Interactor);
	OnQueryUpdated.Broadcast(CachedQueryResult);
}

void UInteractionComponent::BeginInteract()
{
	if (!bEnabled) return;
	
	if (!FocusedActor.IsValid())
	{
		return;
	}

	switch (CachedQueryResult.InputType)
	{
	case EInteractionInputType::Press:
		ExecutePress();
		break;

	case EInteractionInputType::Hold:
		BeginHold(CachedQueryResult.HoldDuration);
		break;

	default:
		ExecutePress();
		break;
	}
}

void UInteractionComponent::EndInteract()
{
	if (!bEnabled) return;
	
	if (bIsHolding)
	{
		ResetHold();
	}
}

void UInteractionComponent::ExecutePress()
{
	if (!FocusedActor.IsValid()) return;

	AActor* Interactor = InteractorActor.Get();
	AActor* Target = FocusedActor.Get();

	if (!IsValid(Interactor) || !IsValid(Target)) return;
	
	IInteractable::Execute_Interact(Target, Interactor);

	// Refresh to correct UI immediately after interaction.
	RefreshQuery();
}

void UInteractionComponent::BeginHold(float DurationSeconds)
{
	if (DurationSeconds <= 0.f)
	{
		ExecutePress();
		return;
	}

	if (!GetWorld()) return;

	bIsHolding = true;
	HoldElapsed = 0.f;
	HoldDuration = DurationSeconds;

	GetWorld()->GetTimerManager().SetTimer(
		HoldTickTimer,
		this,
		&UInteractionComponent::TickHold,
		HoldTickInterval,
		true
	);

	DebugPushSnapshot();
	
	OnHoldProgress.Broadcast(0.f);
}

void UInteractionComponent::TickHold()
{
	if (!bIsHolding)
	{
		return;
	}

	// Focus lost while holding
	if (!FocusedActor.IsValid())
	{
		ResetHold();
		return;
	}

	HoldElapsed += HoldTickInterval;

	OnHoldProgress.Broadcast(GetHoldProgress());

	if (HoldElapsed >= HoldDuration)
	{
		CompleteHold();
	}
	
	DebugPushSnapshot();
}

void UInteractionComponent::CompleteHold()
{
	if (!bIsHolding)
	{
		return;
	}

	ResetHold();
	
	DebugPushSnapshot();
	
	OnHoldCompleted.Broadcast();

	ExecutePress();
}

void UInteractionComponent::ResetHold()
{
	if (!bIsHolding)
	{
		return;
	}

	bIsHolding = false;
	HoldElapsed = 0.f;
	HoldDuration = 0.f;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HoldTickTimer);
	}

	DebugPushSnapshot();
	
	OnHoldProgress.Broadcast(0.f);
	OnHoldReset.Broadcast();
}

void UInteractionComponent::EnableInteraction()
{
	if (bEnabled)
	{
		return;
	}

	bEnabled = true;
	StartFocusScan();

	PerformFocusScan();
}

void UInteractionComponent::DisableInteraction()
{
	if (!bEnabled)
	{
		return;
	}

	bEnabled = false;

	StopFocusScan();
	ResetHold();
	ClearFocus();
}

void UInteractionComponent::ToggleInteraction()
{
	if (bEnabled)
	{
		DisableInteraction();
	}
	else
	{
		EnableInteraction();
	}
}

void UInteractionComponent::ToggleDebugOverlay()
{
	bDebugOverlayEnabled = !bDebugOverlayEnabled;
	if (DebugHelper)
	{
		DebugHelper->SetEnabled(bDebugOverlayEnabled);
		DebugPushSnapshot();
	}
}

void UInteractionComponent::DebugPushSnapshot() const
{
	if (!DebugHelper || !DebugHelper->IsEnabled()) return;

	FInteractionDebugSnapshot S;
	S.Owner = GetOwner();
	S.FocusedActor = FocusedActor;
	S.TraceStart = LastTraceStart;
	S.TraceEnd = LastTraceEnd;
	S.bTraceHit = bLastTraceHit;
	S.HitActor = LastHitActor;
	S.HitResult = LastHitResult;

	S.QueryResult = CachedQueryResult;

	S.bEnabled = bEnabled;
	S.bHolding = bIsHolding;
	S.HoldProgress01 = GetHoldProgress();

	S.ScanInterval = ScanInterval;
	S.TraceDistance = TraceDistance;
	S.TraceRadius = TraceRadius;
	S.bHitWasInteractable = bLastHitWasInteractable;

	DebugHelper->Update(S);
}