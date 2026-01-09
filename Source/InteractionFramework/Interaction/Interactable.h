
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractable
 *
 * Defines the contract for world objects that can be interacted with by an interactor.
 * This interface exposes interaction data, validation, and execution hooks.
 *
 * Implemented by world actors that wish to participate in the interaction system.
 * Called by the InteractionComponent during focus detection and interaction execution.
 *
 * This interface does not handle input, detection, UI presentation, or interaction timing;
 * those concerns are managed externally by the interaction system.
 */

class INTERACTIONFRAMEWORK_API IInteractable
{
	GENERATED_BODY()

public:
};
