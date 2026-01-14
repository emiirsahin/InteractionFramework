#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.h"

class UKeyringComponent;

namespace InteractionUtils
{
	// Returns true if any requirements are missing and fills OutMissingMessages.
	bool BuildMissingMessages(
		const TArray<FInteractionKeyRequirement>& Requirements,
		const UKeyringComponent* Keyring,
		TArray<FText>& OutMissingMessages);

	// Returns true if any requirements are missing.
	bool AreRequirementsMet(
		const TArray<FInteractionKeyRequirement>& Requirements,
		const UKeyringComponent* Keyring);
}
