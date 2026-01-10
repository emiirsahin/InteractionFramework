
#include "InteractionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Interactable.h"
#include "KeyringComponent.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CachedQueryResult = FInteractionQueryResult{};
	CachedQueryResult.bShouldShowPrompt = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	InteractorActor = GetOwner();
	
	StartFocusScan();
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFocusScan();
	CancelHold();
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
	const bool bFound = FindInteractableInView(NewActor);

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
		SetFocused(NewActor);
		return;
	}

	// To keep UI correct if the state of the object changes.
	RefreshQuery();
}

bool UInteractionComponent::GetViewPoint(FVector& OutViewLoc, FRotator& OutViewRot) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	AActor* Owner = InteractorActor.Get();
	if (!IsValid(Owner)) return false;

	// Prefer owner controller viewpoint (For player-controlled pawns).
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (AController* Controller = Pawn->GetController())
		{
			Controller->GetPlayerViewPoint(OutViewLoc, OutViewRot);
			return true;
		}
	}

	// Fallback (single-player case).
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		PC->GetPlayerViewPoint(OutViewLoc, OutViewRot);
		return true;
	}

	return false;
}

bool UInteractionComponent::FindInteractableInView(AActor*& OutActor) const
{
	OutActor = nullptr;

	UWorld* World = GetWorld();
	if (!World) return false;

	FVector ViewLoc;
	FRotator ViewRot;
	if (!GetViewPoint(ViewLoc, ViewRot)) return false;

	const FVector Start = ViewLoc;
	const FVector End = Start + (ViewRot.Vector() * TraceDistance);

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

	if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		OutActor = HitActor;
		return true;
	}

	return false;
}

void UInteractionComponent::SetFocused(AActor* NewActor)
{
	CancelHold();

	AActor* Prev = FocusedActor.Get();
	FocusedActor = NewActor;

	RefreshQuery();
	OnFocusChanged.Broadcast(NewActor, Prev);
}

void UInteractionComponent::ClearFocus()
{
	CancelHold();

	AActor* Prev = FocusedActor.Get();
	FocusedActor = nullptr;

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
		CancelHold();
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
		CancelHold();
		return;
	}

	HoldElapsed += HoldTickInterval;

	OnHoldProgress.Broadcast(GetHoldProgress());

	if (HoldElapsed >= HoldDuration)
	{
		CompleteHold();
	}
}

void UInteractionComponent::CompleteHold()
{
	if (!bIsHolding)
	{
		return;
	}

	CancelHold();
	OnHoldCompleted.Broadcast();

	ExecutePress();
}

void UInteractionComponent::CancelHold()
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

	OnHoldProgress.Broadcast(0.f);
	OnHoldCanceled.Broadcast();
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
	CancelHold();
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