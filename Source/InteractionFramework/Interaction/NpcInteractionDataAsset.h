#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.h"
#include "Engine/DataAsset.h"
#include "NpcInteractionDataAsset.generated.h"

/**
 * Defines one dialogue step/state for an NPC interaction.
 * Each state can require keys (AND logic) and provides a line for both
 * "requirements met" and "requirements missing" outcomes.
 *
 * Note: This is intentionally lightweight and not a full dialogue framework.
 * It exists to demonstrate integration with the interaction system.
 */
USTRUCT(BlueprintType)
struct FNpcDialogueState
{
	GENERATED_BODY()

public:
	/** Unique identifier for this state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC|State")
	FName StateId = NAME_None;

	/** Dialogue line when the interaction is fired while requirements of this state are not met. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC|State")
	FText LineIfMissing;
	/** Dialogue line when the interaction is fired while requirements of this state are met. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC|State")
	FText LineIfMet;

	/** Keys required for this state's "met" outcome (AND logic). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC|State")
	TArray<FInteractionKeyRequirement> RequiredKeys;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC|State")
	float SpeechWidgetVisibleTime;

	bool IsValid() const { return !StateId.IsNone(); }
};

/**
 * Designer-authored definition of how an NPC interaction should be presented to the player.
 * This asset is static (design-time data). Runtime availability is determined by the interactable.
 */
UCLASS(BlueprintType)
class INTERACTIONFRAMEWORK_API UNpcInteractionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Prompt displayed in the interaction UI widget (In our case, it will always remain "Talk"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC")
	FText PromptText;

	/** Default starting state id. Must exist in States. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC")
	FName DefaultStateId = NAME_None;

	/** List of dialogue states. StateId must be unique. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="NPC")
	TArray<FNpcDialogueState> States;

public:
	/** Finds a state by id. Returns null if not found. */
	const FNpcDialogueState* FindStateById(FName StateId) const
	{
		for (const FNpcDialogueState& State : States)
		{
			if (State.StateId == StateId)
			{
				return &State;
			}
		}
		return nullptr;
	}

	/** Returns the default state definition (may be null if none defined). */
	FName GetDefaultStateId() const
	{
		// If DefaultStateId is set, try it first
		if (!DefaultStateId.IsNone())
		{
			if (const FNpcDialogueState* Found = FindStateById(DefaultStateId))
			{
				return Found->StateId;
			}
		}

		// This happens by default through data validation, but still a good check to keep
		// Otherwise fall back to first entry
		return (States.Num() > 0) ? States[0].StateId : NAME_None;
	}
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
