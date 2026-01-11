#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.generated.h"

/**
 * High-level availability state for an interaction target.
 * Note: Interaction may still be attempted even when unavailable; this is primarily for UI/presentation.
 */
UENUM(BlueprintType)
enum class EInteractionAvailability : uint8
{
	Available     UMETA(DisplayName="Available"),
	Unavailable   UMETA(DisplayName="Unavailable")
};

UENUM(BlueprintType)
enum class EInteractionInputType : uint8
{
	Press UMETA(DisplayName="Press"),
	Hold  UMETA(DisplayName="Hold")
};

/**
 * A single key requirement entry.
 */
USTRUCT(BlueprintType)
struct FInteractionKeyRequirement
{
	GENERATED_BODY()

public:
	/** Key identifier required to successfully interact. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Requirements")
	FName KeyId;

	/** Display label for UI (e.g., "Red Keycard"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Requirements")
	FText DisplayText;

	/** Message shown when this key is missing (e.g., "The Red Keycard is missing"). */	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Requirements")
	FText MissingMessage;
};

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

	/** State is valid if it has a non-None id. */
	bool IsValid() const { return !StateId.IsNone(); }
};

/**
 * Result of querying an interactable's current interaction state (primarily for UI/presentation).
 */
USTRUCT(BlueprintType)
struct FInteractionQueryResult
{
	GENERATED_BODY()

public:
	/**
	 * Whether the interaction prompt should be shown at all.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bShouldShowPrompt = true;
	
	/** Final prompt text to show in the UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|UI")
	FText PromptText;

	/** How the interaction is performed (Press / Hold). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Input")
	EInteractionInputType InputType = EInteractionInputType::Press;

	/** Hold duration (only relevant when InputType == Hold). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Input")
	float HoldDuration = 0.f;

	/**
	 * Ordered list of messages describing unmet requirements (e.g., "Requires Red Keycard").
	 * Empty when Availability is Available.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TArray<FText> UnmetRequirementMessages;

	bool ShouldShowPrompt() const
	{
		return bShouldShowPrompt;
	}
	
	static FInteractionQueryResult Make(
	bool bInShouldShowPrompt,
	const FText& InPromptText,
	EInteractionInputType InInputType = EInteractionInputType::Press,
	float InHoldDuration = 0.f,
	const TArray<FText>& InUnmetMessages = TArray<FText>())
	{
		FInteractionQueryResult Result;
		Result.bShouldShowPrompt = bInShouldShowPrompt;
		Result.PromptText = InPromptText;
		Result.InputType = InInputType;
		Result.HoldDuration = (InInputType == EInteractionInputType::Hold) ? InHoldDuration : 0.f;
		Result.UnmetRequirementMessages = InUnmetMessages;
		return Result;
	}

	bool IsAvailable() const
	{
		return UnmetRequirementMessages.Num() == 0;
	}
};
