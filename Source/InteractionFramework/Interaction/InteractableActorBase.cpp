
#include "InteractableActorBase.h"
#include "InteractionDataAsset.h"
#include "InteractionTypes.h"
#include "KeyringComponent.h"

AInteractableActorBase::AInteractableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractableActorBase::NotifyInteractionStateChanged()
{
	OnInteractionStateChanged.Broadcast();
}

UInteractionDataAsset* AInteractableActorBase::GetInteractionData_Implementation() const
{
	return InteractionData.Get();
}

FInteractionQueryResult AInteractableActorBase::QueryInteraction_Implementation(AActor* Interactor) const
{
	FInteractionQueryResult Result{};

	// No data means no prompt
	if (!InteractionData || !InteractionData->bShouldShowPrompt)
	{
		Result.bShouldShowPrompt = false;
		return Result;
	}

	// Copy UI info from data asset
	Result.bShouldShowPrompt = InteractionData->bShouldShowPrompt;
	Result.PromptText        = InteractionData->PrimaryPromptText;
	Result.InputType         = InteractionData->InputType;
	Result.HoldDuration      = InteractionData->HoldDuration;

	// Only do requirement check if there are any requirements
	if (InteractionData->RequiredKeys.Num() > 0)
	{
		GetMissingRequirementMessages(Interactor, Result.UnmetRequirementMessages);
	}

	return Result;
}

bool AInteractableActorBase::GetMissingRequirementMessages(AActor* Interactor, TArray<FText>& OutMissingMessages) const
{
	OutMissingMessages.Reset();

	if (!InteractionData)
	{
		return false;
	}

	const TArray<FInteractionKeyRequirement>& Reqs = InteractionData->RequiredKeys;
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
	// Always allow the attempt, success depends on requirements
	TArray<FText> Missing;
	const bool bHasMissing = (InteractionData && InteractionData->RequiredKeys.Num() > 0)
		? GetMissingRequirementMessages(Interactor, Missing)
		: false;

	if (bHasMissing)
	{
		K2_OnInteractUnavailable(Interactor);
		return;
	}

	K2_OnInteractAvailable(Interactor);
}

