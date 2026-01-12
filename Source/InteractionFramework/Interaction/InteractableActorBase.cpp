
#include "InteractableActorBase.h"
#include "InteractionDataAsset.h"
#include "InteractionTypes.h"
#include "KeyringComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogInteractionFramework, Log, All);

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

UInteractionDataAsset* AInteractableActorBase::GetInteractionData_Implementation() const
{
	return InteractionData.Get();
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

	for (const FInteractionKeyRequirement& Req : Reqs)
	{
		if (Req.KeyId.IsNone())
		{
			continue;
		}

		const bool bHasKey = (Keyring && Keyring->HasKey(Req.KeyId));
		if (!bHasKey)
		{
			OutMissingMessages.Add(Req.MissingMessage);
		}
	}

	return OutMissingMessages.Num() > 0;
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


void AInteractableActorBase::LogCachedStateDefNull()
{
	UE_LOG(LogInteractionFramework, Error, TEXT("CachedStateDef is null."));
}