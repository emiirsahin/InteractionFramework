
#include "InteractableActorBase.h"

#include "InteractionUtils.h"
#include "Interaction/Data/InteractionDataAsset.h"
#include "Interaction/Data/InteractionTypes.h"
#include "KeyringComponent.h"

AInteractableActorBase::AInteractableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableActorBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeInteractionState();
}

void AInteractableActorBase::InitializeInteractionState()
{
	if (!InteractionData)
	{
		return;
	}
	
	// If state was set in-editor, cache it.

	if (!SetInteractionState(CurrentStateId))
	{
		if (!SetInteractionState(InteractionData->GetDefaultStateId()))
		{
			LogCachedStateDefNull();
		}
	}
}

bool AInteractableActorBase::SetInteractionState(FName NewStateId)
{
	if (!InteractionData) return false;

	if (!NewStateId.IsNone())
	{
		if (CurrentStateId == NewStateId && CurrentState.IsValid())
		{
			return true;
		}

		if (!InteractionData) return false; // Can not check if the provided new state id exists.

		if (CacheStateFromId(NewStateId))
		{
			return true;
		}
	}
	
	return false;
}

FInteractionQueryResult AInteractableActorBase::QueryInteraction_Implementation(AActor* Interactor) const
{
	FInteractionQueryResult Result{};

	// No data means no prompt
	if (!InteractionData || !CurrentState.IsValid())
	{
		Result.bShouldShowPrompt = false;
		return Result;
	}

	// Copy UI info from data asset
	Result.bShouldShowPrompt = InteractionData->ShouldShowPromptForState(CurrentState);
	Result.PromptText        = CurrentState.PromptText;
	Result.InputType         = CurrentState.InputType;
	Result.HoldDuration      = CurrentState.HoldDuration;

	// Only do requirement check if there are any requirements
	if (CurrentState.RequiredKeys.Num() > 0)
	{
		GetMissingRequirementMessages(Interactor, Result.UnmetRequirementMessages);
		Result.UnmetRequirementNumber = Result.UnmetRequirementMessages.Num();
	}

	return Result;
}

bool AInteractableActorBase::GetMissingRequirementMessages(AActor* Interactor, TArray<FText>& OutMissingMessages) const
{
	OutMissingMessages.Reset();

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

	return InteractionUtils::BuildMissingMessages(Reqs, Keyring, OutMissingMessages);
}


void AInteractableActorBase::Interact_Implementation(AActor* Interactor)
{
	if (!InteractionData || !CurrentState.IsValid())
	{
		LogCachedStateDefNull();
		return;
	}
	// Always allow the attempt, success depends on requirements
	TArray<FText> Missing;
	const bool bHasMissing = GetMissingRequirementMessages(Interactor, Missing);

	if (bHasMissing)
	{
		K2_OnInteractUnavailable(Interactor, Missing);
		return;
	}

	K2_OnInteractAvailable(Interactor);
}

bool AInteractableActorBase::CacheStateFromId(FName StateId)
{
	if (!InteractionData || StateId.IsNone())
	{
		return false;
	}

	if (const FInteractionStateDefinition* Found = InteractionData->FindStateById(StateId))
	{
		CurrentState = *Found;
		CurrentStateId = StateId;
		return true;
	}

	return false;
}