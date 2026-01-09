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
	
	/** Whether the interaction is currently available (for UI). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	EInteractionAvailability Availability = EInteractionAvailability::Available;

	/**
	 * Ordered list of messages describing unmet requirements (e.g., "Requires Red Keycard").
	 * Empty when Availability is Available.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TArray<FText> UnmetRequirementMessages;

	bool IsAvailable() const
	{
		return Availability == EInteractionAvailability::Available;
	}

	bool ShouldShowPrompt() const
	{
		return bShouldShowPrompt;
	}
	
	static FInteractionQueryResult Make(
		const bool bInShouldShowPrompt,
		const EInteractionAvailability InAvailability,
		const TArray<FText>& InUnmetRequirementMessages = TArray<FText>())
	{
		FInteractionQueryResult Result;
		Result.bShouldShowPrompt = bInShouldShowPrompt;
		Result.Availability = InAvailability;

		// Defensive: only carry unmet messages when unavailable.
		if (InAvailability == EInteractionAvailability::Unavailable)
		{
			Result.UnmetRequirementMessages = InUnmetRequirementMessages;
		}
		else
		{
			Result.UnmetRequirementMessages.Reset();
		}

		return Result;
	}
};
