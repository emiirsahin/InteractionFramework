#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InteractionTypes.h"
#include "InteractionDataAsset.generated.h"

/** Prompt visibility for an interaction object */
UENUM(BlueprintType)
enum class EInteractionPromptOverride : uint8
{
	UsePerState UMETA(DisplayName="Use Per-State"),
	ForceShow   UMETA(DisplayName="Force Show"),
	ForceHide   UMETA(DisplayName="Force Hide"),
};

/**
 * Describes a single interaction "state" for an interactable object.
 * The owning actor keeps current StateId is.
 */
USTRUCT(BlueprintType)
struct FInteractionStateDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State")
	FName StateId = NAME_None;

	/** What the interaction prompt says for this state (e.g. Open, Close, Unlock). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State")
	FText PromptText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State")
	EInteractionInputType InputType = EInteractionInputType::Press;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State", meta=(ClampMin="0.0"))
	float HoldDuration = 0.0f;

	/** Requirements for this state (AND logic between keys). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State")
	TArray<FInteractionKeyRequirement> RequiredKeys;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State")
	bool bShouldShowPrompt = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|State")
	bool bShouldShowRequirements = true;

	/** State is valid if it has a non-None id. */
	bool IsValid() const { return !StateId.IsNone(); }
};

/**
 * UInteractionDataAsset
 *
 * Designer-authored definition of how an interaction should be presented to the player.
 * This asset is static (design-time data). Runtime availability is determined by the interactable.
 */
UCLASS(BlueprintType)
class INTERACTIONFRAMEWORK_API UInteractionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Name of the interactable object (Door, Chest, NPC, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	FText DisplayName;

	/** How prompts should be shown across all states. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	EInteractionPromptOverride PromptOverridePolicy = EInteractionPromptOverride::UsePerState;

	/**
	 * Default state used when the actor has not set a state or the requested state isn't found.
	 * If None, and the State list is not empty, the first entry in States is used.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	FName DefaultStateId = NAME_None;

	/** All possible interaction states for this interactable type. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction")
	TArray<FInteractionStateDefinition> States;

public:
	/** Finds a state by id. Returns null if not found. */
	const FInteractionStateDefinition* FindStateById(FName StateId) const
	{
		for (const FInteractionStateDefinition& State : States)
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
			if (const FInteractionStateDefinition* Found = FindStateById(DefaultStateId))
			{
				return Found->StateId;
			}
		}

		// This happens by default through data validation, but still a good check to keep
		// Otherwise fall back to first entry
		return (States.Num() > 0) ? States[0].StateId : NAME_None;
	}
	
	bool ShouldShowPromptForState(const FInteractionStateDefinition& State) const
	{
		switch (PromptOverridePolicy)
		{
		case EInteractionPromptOverride::ForceShow:
			return true;
		case EInteractionPromptOverride::ForceHide:
			return false;
		case EInteractionPromptOverride::UsePerState:
		default:
			return State.bShouldShowPrompt;
		}
	}
	
	static bool IsHoldInteraction(const FInteractionStateDefinition& State)
	{
		return State.InputType == EInteractionInputType::Hold;
	}
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
