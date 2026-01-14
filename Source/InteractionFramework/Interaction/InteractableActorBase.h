
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "InteractionDataAsset.h"
#include "InteractableActorBase.generated.h"

class UInteractionDataAsset;

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

	virtual void BeginPlay() override;

	// IInteractable
	virtual FInteractionQueryResult QueryInteraction_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool SetInteractionState(FName NewStateId);
	
public:
	/** Static configuration of this interaction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UInteractionDataAsset> InteractionData;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	FName CurrentStateId = NAME_None;

	/** Copy of the current state. */
	UPROPERTY(Transient)
	FInteractionStateDefinition CurrentState;
	
protected:
	void InitializeInteractionState();
	
	/** Called when Interact() is invoked and the interaction is currently available. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void K2_OnInteractAvailable(AActor* Interactor);

	/** Called when Interact() is invoked but the interaction is currently unavailable. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction")
	void K2_OnInteractUnavailable(AActor* Interactor, const TArray<FText>& MissingMessages);

	/** Helper to get a list of missing requirement messages for the given keyring. */
	bool GetMissingRequirementMessages(AActor* Interactor, TArray<FText>& OutMissingMessages) const;

	bool CacheStateFromId(FName StateId);
	
	static void LogCachedStateDefNull()
	{
		UE_LOG(LogInteractionFramework, Error, TEXT("CachedStateDef is null."));
	}
};
