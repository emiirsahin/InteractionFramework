#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interaction/InteractionTypes.h"
#include "InteractionPromptWidget.generated.h"

/**
 * Base class for the interaction prompt UI.
 * BP subclass is responsible for presentation; C++ pushes state via events.
 */
UCLASS(Abstract)
class INTERACTIONFRAMEWORK_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called when the focused interactable query data changes. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction|UI")
	void BP_SetQueryResult(const FInteractionQueryResult& Query);

	/** Called while holding (0..1). */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction|UI")
	void BP_SetHoldProgress(float NormalizedProgress);

	/** Show/hide the prompt widget. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction|UI")
	void BP_SetPromptVisible(bool bVisible);
};
