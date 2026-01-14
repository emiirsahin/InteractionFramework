#include "InteractionUtils.h"
#include "KeyringComponent.h"

bool InteractionUtils::BuildMissingMessages(
	const TArray<FInteractionKeyRequirement>& Requirements,
	const UKeyringComponent* Keyring,
	TArray<FText>& OutMissingMessages)
{
	OutMissingMessages.Reset();

	if (Requirements.Num() == 0)
	{
		return false;
	}

	for (const FInteractionKeyRequirement& Req : Requirements)
	{
		if (Req.KeyId.IsNone())
		{
			continue;
		}

		const bool bHasKey = (Keyring && Keyring->HasKey(Req.KeyId));
		if (!bHasKey)
		{
			OutMissingMessages.Add(Req.MissingMessage);
		}
	}

	return OutMissingMessages.Num() > 0;
}
