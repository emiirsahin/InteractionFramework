#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.generated.h"

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bShouldShowPrompt = true;
	
	/** Final prompt text to show in the UI. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|UI")
	FText PromptText;

	/** How the interaction is performed (Press / Hold). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|Input")
	EInteractionInputType InputType = EInteractionInputType::Press;

	/** Hold duration (only relevant when InputType == Hold). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|Input")
	float HoldDuration = 0.f;

	/**
	 * Ordered list of messages describing unmet requirements (e.g., "Requires Red Keycard").
	 * Empty when Availability is Available.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Interaction")
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
