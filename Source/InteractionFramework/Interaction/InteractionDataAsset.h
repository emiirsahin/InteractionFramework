#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InteractionDataAsset.generated.h"

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Requirements")
	FText MissingMessage;
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
	/** Optional label for the target ("Door"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|UI")
	FText DisplayName;

	/** Primary prompt text shown to the player ("Open"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|UI")
	FText PromptText;

	/** Whether the interaction is performed by a press or a hold. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Input")
	EInteractionInputType InputType = EInteractionInputType::Press;

	/** Hold duration in seconds (only used when InputType == Hold). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Input",
		meta=(ClampMin="0.0", EditCondition="InputType == EInteractionInputType::Hold", EditConditionHides))
	float HoldDuration = 0.5f;

	/**
	 * If true, UI may show availability state / missing requirements (e.g., greyed out + requirements list).
	 * If false, UI should avoid revealing whether the interaction is currently available.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|UI")
	bool bAllowUIToShowAvailability = true;

	/**
	 * Keys required for successful interaction (AND logic).
	 * The list order is preserved for UI presentation.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Requirements")
	TArray<FInteractionKeyRequirement> RequiredKeys;

	UFUNCTION(BlueprintPure, Category="Interaction|Input")
	bool IsHoldInteraction() const { return InputType == EInteractionInputType::Hold; }
};
