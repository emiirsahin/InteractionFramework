
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

	if (!SetInteractionStateById(CurrentStateId))
	{
		if (const FInteractionStateDefinition* DefaultState = InteractionData->GetDefaultState())
		{
			SetInteractionStateByDefinition(*DefaultState);
		}
		else
		{
			LogCachedStateDefNull();
		}
	}
}

bool AInteractableActorBase::SetInteractionStateById(FName NewStateId)
{
	return SetInteractionStateInternal(NewStateId, nullptr);
}

bool AInteractableActorBase::SetInteractionStateByDefinition(const FInteractionStateDefinition& State)
{
	return SetInteractionStateInternal(NAME_None, &State);
}

bool AInteractableActorBase::SetInteractionStateInternal(FName NewStateId, const FInteractionStateDefinition* State)
{
	if (State)
	{
		CurrentStateId = State->StateId;
		CachedStateDef = State;
		return true;
	}

	if (!NewStateId.IsNone())
	{
		if (CurrentStateId == NewStateId && CachedStateDef)
		{
			return true;
		}

		if (!InteractionData) return false; // Can not check if the provided new state id exists.

		if (const FInteractionStateDefinition* NewState = InteractionData->FindStateById(NewStateId))
		{
			CurrentStateId = NewState->StateId;
			CachedStateDef = NewState;
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
	if (!InteractionData || !CachedStateDef)
	{
		Result.bShouldShowPrompt = false;
		return Result;
	}

	// Copy UI info from data asset
	Result.bShouldShowPrompt = InteractionData->ShouldShowPromptForState(*CachedStateDef);
	Result.PromptText        = CachedStateDef->PromptText;
	Result.InputType         = CachedStateDef->InputType;
	Result.HoldDuration      = CachedStateDef->HoldDuration;

	// Only do requirement check if there are any requirements
	if (CachedStateDef->RequiredKeys.Num() > 0)
	{
		GetMissingRequirementMessages(Interactor, Result.UnmetRequirementMessages);
	}

	return Result;
}

bool AInteractableActorBase::GetMissingRequirementMessages(AActor* Interactor, TArray<FText>& OutMissingMessages) const
{
	OutMissingMessages.Reset();

	if (!CachedStateDef)
	{
		return false;
	}
	
	const TArray<FInteractionKeyRequirement>& Reqs = CachedStateDef->RequiredKeys;
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
	if (!InteractionData || !CachedStateDef)
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

void AInteractableActorBase::LogCachedStateDefNull()
{
	UE_LOG(LogInteractionFramework, Error, TEXT("CachedStateDef is null."));
}