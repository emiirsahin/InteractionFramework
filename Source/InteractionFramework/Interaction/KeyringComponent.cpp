
#include "KeyringComponent.h"

UKeyringComponent::UKeyringComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UKeyringComponent::HasKey(FName KeyId) const
{
	return !KeyId.IsNone() && OwnedKeys.Contains(KeyId);
}

bool UKeyringComponent::AddKey(FName KeyId)
{
	if (KeyId.IsNone())
	{
		return false;
	}

	const int32 PrevNum = OwnedKeys.Num();
	OwnedKeys.Add(KeyId);
	return OwnedKeys.Num() != PrevNum;
}

bool UKeyringComponent::RemoveKey(FName KeyId)
{
	if (KeyId.IsNone())
	{
		return false;
	}

	return OwnedKeys.Remove(KeyId) > 0;
}

bool UKeyringComponent::HasAllKeys(const TArray<FName>& RequiredKeys) const
{
	for (const FName& KeyId : RequiredKeys)
	{
		if (KeyId.IsNone())
		{
			continue;
		}

		if (!OwnedKeys.Contains(KeyId))
		{
			return false;
		}
	}

	return true;
}