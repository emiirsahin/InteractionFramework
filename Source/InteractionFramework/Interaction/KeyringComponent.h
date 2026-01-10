
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeyringComponent.generated.h"

/**
 * UKeyringComponent
 *
 * Stores a set of "keys" (IDs) owned by an actor (The player).
 * Has a hash set for fast checks.
 *
 * Intended to be queried by interactables during QueryInteraction/Interact
 * to determine whether key-requirement interactions are available.
 *
 * Keys can be anything, a quest completion, a milestone, a collectible, actual keys, etc.
 */
UCLASS(ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class INTERACTIONFRAMEWORK_API UKeyringComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeyringComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Keyring")
	bool HasKey(FName KeyId) const;

	UFUNCTION(BlueprintCallable, Category="Keyring")
	bool AddKey(FName KeyId);

	UFUNCTION(BlueprintCallable, Category="Keyring")
	bool RemoveKey(FName KeyId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Keyring")
	bool HasAllKeys(const TArray<FName>& RequiredKeys) const;

protected:
	UPROPERTY(VisibleAnywhere, Category="Keyring")
	TSet<FName> OwnedKeys;
};
