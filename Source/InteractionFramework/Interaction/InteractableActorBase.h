
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractableActorBase.generated.h"

class UInteractionDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionStateChanged);

/**
 * AInteractableActorBase
 *
 * Base class for world actors participating in the interaction framework.
 * Implements IInteractable and provides default behavior:
 * - Holds InteractionData data asset
 * - Creates a FInteractionQueryResult for the interactor InteractionComponent
 * - Sends interaction attempts to Blueprint hooks (available/unavailable)
 */
UCLASS(Abstract, BlueprintType)
class INTERACTIONFRAMEWORK_API AInteractableActorBase
	: public AActor
	, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableActorBase();

	/** Static configuration of this interaction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UInteractionDataAsset> InteractionData;

	/**
	 * Fired when this interactable's runtime state changes in a way that may
	 * affect QueryInteraction results (e.g., unlocked, enabled, opened/closed).
	 */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionStateChanged OnInteractionStateChanged;

protected:
	/** Notify listeners that interaction state has changed. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void NotifyInteractionStateChanged();

public:
	// IInteractable
	virtual UInteractionDataAsset* GetInteractionData_Implementation() const override;
	virtual FInteractionQueryResult QueryInteraction_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	/** Called when Interact() is invoked and the interaction is currently available. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void K2_OnInteractAvailable(AActor* Interactor);

	/** Called when Interact() is invoked but the interaction is currently unavailable. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void K2_OnInteractUnavailable(AActor* Interactor);

	/** Helper to get a list of missing requirement messages for the given keyring. */
	bool GetMissingRequirementMessages(AActor* Interactor, TArray<FText>& OutMissingMessages) const;
};
