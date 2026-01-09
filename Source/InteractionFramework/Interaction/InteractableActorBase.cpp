
#include "Interaction/InteractableActorBase.h"
#include "Interaction/InteractionDataAsset.h"
#include "Interaction/InteractionTypes.h"

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
	// No interaction data means hide prompt entirely.
	if (!InteractionData)
	{
		return FInteractionQueryResult::Make(
			false,
			FText::GetEmpty()
		);
	}

	// Resolve UI-facing fields from the DataAsset.
	const FText PromptText = InteractionData->PrimaryPromptText;
	const EInteractionInputType InputType = InteractionData->InputType;
	const float HoldDuration = InteractionData->HoldDuration;
	
	if (InteractionData->RequiredKeys.Num() == 0)
	{
		return FInteractionQueryResult::Make(
			true,
			PromptText,
			InputType,
			HoldDuration
		);
	}

	// TODO: Replace this temporary behavior with keyring checks against the Interactor.
	// For now treat all requirements as unmet so UI flow can be tested for the unavailable case.
	
	TArray<FText> MissingMessages;
	MissingMessages.Reserve(InteractionData->RequiredKeys.Num());

	for (const FInteractionKeyRequirement& Requirement : InteractionData->RequiredKeys)
	{
		MissingMessages.Add(Requirement.MissingMessage);
	}

	return FInteractionQueryResult::Make(
		true,
		PromptText,
		InputType,
		HoldDuration,
		MissingMessages
	);
}

void AInteractableActorBase::Interact_Implementation(AActor* Interactor)
{
	// Interact is always callable; route behavior based on current query result.
	const FInteractionQueryResult QueryResult = QueryInteraction_Implementation(Interactor);

	if (QueryResult.IsAvailable())
	{
		K2_OnInteractAvailable(Interactor);
	}
	else
	{
		K2_OnInteractUnavailable(Interactor);
	}
}
