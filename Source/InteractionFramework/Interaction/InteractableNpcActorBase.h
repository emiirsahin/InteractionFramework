#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "NpcInteractionDataAsset.h"
#include "InteractableNpcActorBase.generated.h"

class UWidgetComponent;
class UKeyringComponent;

UCLASS()
class INTERACTIONFRAMEWORK_API AInteractableNpcActorBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractableNpcActorBase();
	
	// BP
	UFUNCTION(BlueprintImplementableEvent, Category="NPC|Interaction")
	void K2_OnTalkRequirementsUnmet(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category="NPC|Interaction")
	void K2_OnTalkRequirementsMet(AActor* Interactor);

	/** Set state by id (BP can call this on success). */
	UFUNCTION(BlueprintCallable, Category="NPC")
	bool SetNpcState(FName NewStateId);

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC")
	TObjectPtr<UNpcInteractionDataAsset> NpcData = nullptr;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="NPC")
	FName CurrentStateId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC|UI")
	TObjectPtr<UWidgetComponent> SpeechBubbleComponent = nullptr;

	UPROPERTY(Transient)
	FNpcDialogueState CurrentState;

	FTimerHandle BubbleHideTimer;

protected:
	virtual void BeginPlay() override;

	// Interface
	virtual FInteractionQueryResult QueryInteraction_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	void InitializeNpcState();
	bool CacheStateFromId(FName StateId);
	bool GetMissingRequirements(AActor* Interactor) const;

	void ShowBubble(const FText& Line, float Duration);
	void HideBubble();
	
	static void LogCachedStateDefNull()
	{
		UE_LOG(LogInteractionFramework, Error, TEXT("CachedStateDef is null."));
	}
};
