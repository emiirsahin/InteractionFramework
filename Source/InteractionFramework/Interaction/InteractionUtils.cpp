#include "InteractionUtils.h"
#include "KeyringComponent.h"

bool InteractionUtils::AreRequirementsMet(
	const TArray<FInteractionKeyRequirement>& Requirements,
	const UKeyringComponent* Keyring)
{
	for (const FInteractionKeyRequirement& Req : Requirements)
	{
		if (!Req.KeyId.IsNone() && !(Keyring && Keyring->HasKey(Req.KeyId)))
		{
			return true;
		}
	}
	return false;
}

bool InteractionUtils::BuildMissingMessages(
	const TArray<FInteractionKeyRequirement>& Requirements,
	const UKeyringComponent* Keyring,
	TArray<FText>& OutMissingMessages)
{
	OutMissingMessages.Reset();

	for (const FInteractionKeyRequirement& Req : Requirements)
	{
		if (!Req.KeyId.IsNone() && !(Keyring && Keyring->HasKey(Req.KeyId)))
		{
			OutMissingMessages.Add(Req.MissingMessage);
		}
	}

	return OutMissingMessages.Num() > 0;
}
