
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionTypes.h"
#include "InteractionDataAsset.h"
#include "Interactable.generated.h"

UINTERFACE(BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractable
 *
 * Defines the contract for world objects that can be interacted with by an interactor.
 * This interface exposes interaction data, validation, and execution hooks.
 *
 * Implemented by world actors that wish to participate in the interaction system.
 * Called by the InteractionComponent during focus detection and interaction execution.
 *
 * This interface does not handle input, detection, UI presentation, or interaction timing;
 * those concerns are managed externally by the interaction system, and mainly the interaction component.
 */

class INTERACTIONFRAMEWORK_API IInteractable
{
	GENERATED_BODY()

public:
	/** Returns the data describing how this interaction is presented in UI. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	UInteractionDataAsset* GetInteractionData() const;

	/**
	 * Returns current interaction state for UI presentation.
	 * Must stay lightweight and safe.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FInteractionQueryResult QueryInteraction(AActor* Interactor) const;

	/**
	 * Execute an interaction request, result depends on this object.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	void Interact(AActor* Interactor);

	/** Cosmetic hook called when this object becomes the focused interaction target. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Interaction")
	void OnFocusStart(AActor* Interactor);

	/** Cosmetic hook called when this object is no longer the focused interaction target. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Interaction")
	void OnFocusEnd(AActor* Interactor);
};