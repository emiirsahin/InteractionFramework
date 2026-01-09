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

	/** Optional reason shown when Availability is Unavailable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText UnavailableReason;

	bool IsAvailable() const
	{
		return Availability == EInteractionAvailability::Available;
	}

	bool ShouldShowPrompt() const
	{
		return bShouldShowPrompt;
	}
	
	static FInteractionQueryResult Make(
		const bool bInShowPrompt,
		const EInteractionAvailability InAvailability,
		const FText& InUnavailableReason = FText::GetEmpty())
	{
		FInteractionQueryResult Result;
		Result.bShouldShowPrompt = bInShowPrompt;
		Result.Availability = InAvailability;
		Result.UnavailableReason =
			(InAvailability == EInteractionAvailability::Unavailable) ? InUnavailableReason : FText::GetEmpty();
		return Result;
	}
};
