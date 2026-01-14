
#include "InteractableNpcActorBase.h"

#include "InteractionUtils.h"
#include "KeyringComponent.h"
#include "NpcSpeechBubbleWidget.h"
#include "Components/WidgetComponent.h"

AInteractableNpcActorBase::AInteractableNpcActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SpeechBubbleComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SpeechBubble"));
	SpeechBubbleComponent->SetupAttachment(RootComponent);

	SpeechBubbleComponent->SetWidgetSpace(EWidgetSpace::Screen); // always faces camera
	SpeechBubbleComponent->SetDrawAtDesiredSize(true);
	SpeechBubbleComponent->SetVisibility(false);
}

void AInteractableNpcActorBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeNpcState();
}

void AInteractableNpcActorBase::InitializeNpcState()
{
	if (!NpcData)
	{
		return;
	}
	
	// If state was set in-editor, cache it.

	if (!SetNpcState(CurrentStateId))
	{
		if (!SetNpcState(NpcData->GetDefaultStateId()))
		{
			LogCachedStateDefNull();
		}
	}
}

bool AInteractableNpcActorBase::SetNpcState(FName NewStateId)
{
	if (!NpcData) return false;

	if (!NewStateId.IsNone())
	{
		if (CurrentStateId == NewStateId && CurrentState.IsValid())
		{
			return true;
		}

		if (!NpcData) return false; // Can not check if the provided new state id exists.

		if (CacheStateFromId(NewStateId))
		{
			return true;
		}
	}
	
	return false;
}

FInteractionQueryResult AInteractableNpcActorBase::QueryInteraction_Implementation(AActor* Interactor) const
{
	FInteractionQueryResult Result{};
	Result.bShouldShowPrompt = true;
	Result.InputType = EInteractionInputType::Press;
	Result.HoldDuration = 0.f;
	Result.UnmetRequirementMessages.Reset(); // NPC doesn't show these

	if (NpcData && !NpcData->PromptText.IsEmpty())
		Result.PromptText = NpcData->PromptText;
	else
		Result.PromptText = FText::FromString(TEXT("Talk"));

	return Result;
}

bool AInteractableNpcActorBase::GetMissingRequirements(AActor* Interactor) const
{
	if (!CurrentState.IsValid())
	{
		return false;
	}
	
	const TArray<FInteractionKeyRequirement>& Reqs = CurrentState.RequiredKeys;
	if (Reqs.Num() == 0)
	{
		return false;
	}

	const UKeyringComponent* Keyring =
		Interactor ? Interactor->FindComponentByClass<UKeyringComponent>() : nullptr;

	return InteractionUtils::AreRequirementsMet(Reqs, Keyring);
}

void AInteractableNpcActorBase::Interact_Implementation(AActor* Interactor)
{
	if (!NpcData || CurrentState.StateId.IsNone())
	{
		return;
	}

	const bool bMet = !GetMissingRequirements(Interactor);

	const FText LineToShow = bMet ? CurrentState.LineIfMet : CurrentState.LineIfMissing;
	const float Duration = CurrentState.SpeechWidgetVisibleTime;

	if (!LineToShow.IsEmpty())
	{
		ShowBubble(LineToShow, Duration);
	}

	if (bMet)
	{
		K2_OnTalkRequirementsMet(Interactor);
	}
	else
	{
		K2_OnTalkRequirementsUnmet(Interactor);
	}
}

void AInteractableNpcActorBase::ShowBubble(const FText& Line, float Duration)
{
	if (!SpeechBubbleComponent) return;

	if (UNpcSpeechBubbleWidget* W = Cast<UNpcSpeechBubbleWidget>(SpeechBubbleComponent->GetUserWidgetObject()))
	{
		W->SetLineText(Line);
	}

	SpeechBubbleComponent->SetVisibility(true);

	// Restart timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BubbleHideTimer);

		if (Duration > 0.f)
		{
			World->GetTimerManager().SetTimer(
				BubbleHideTimer,
				this,
				&AInteractableNpcActorBase::HideBubble,
				Duration,
				false
			);
		}
	}
}

void AInteractableNpcActorBase::HideBubble()
{
	if (!SpeechBubbleComponent) return;

	if (UNpcSpeechBubbleWidget* W = Cast<UNpcSpeechBubbleWidget>(SpeechBubbleComponent->GetUserWidgetObject()))
	{
		W->ClearLineText();
	}

	SpeechBubbleComponent->SetVisibility(false);
}

bool AInteractableNpcActorBase::CacheStateFromId(FName StateId)
{
	if (!NpcData || StateId.IsNone())
	{
		return false;
	}

	if (const FNpcDialogueState* Found = NpcData->FindStateById(StateId))
	{
		CurrentState = *Found;
		CurrentStateId = StateId;
		return true;
	}

	return false;
}