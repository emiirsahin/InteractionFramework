#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionTypes.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFocusChanged, AActor*, NewFocusedActor, AActor*, PreviousFocusedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryUpdated, const FInteractionQueryResult&, QueryResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoldProgress, float, NormalizedProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoldReset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoldCompleted);

class IInteractable;
/**
 * UInteractionComponent
 *
 * Timer-driven focus detection and interaction execution (press/hold).
 * The component depends only on the IInteractable interface.
 *
 * UI should listen to OnQueryUpdated and OnHoldProgress.
 * QueryInteraction is called whenever focus changes and whenever the player interacts with the object
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INTERACTIONFRAMEWORK_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Events
	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnFocusChanged OnFocusChanged;

	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnQueryUpdated OnQueryUpdated;

	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnHoldProgress OnHoldProgress;

	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnHoldReset OnHoldReset;

	UPROPERTY(BlueprintAssignable, Category="Interaction|Events")
	FOnHoldCompleted OnHoldCompleted;

	// Input entry points
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void BeginInteract();

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void EndInteract();

	// UI getters
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UFUNCTION(BlueprintPure, Category="Interaction")
	const FInteractionQueryResult& GetCachedQueryResult() const { return CachedQueryResult; }

	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsHolding() const { return bIsHolding; }

	UFUNCTION(BlueprintPure, Category="Interaction")
	float GetHoldProgress() const;

	// Performance
	UFUNCTION(BlueprintCallable, Category="Interaction|Debug")
	void EnableInteraction();

	UFUNCTION(BlueprintCallable, Category="Interaction|Debug")
	void DisableInteraction();

	UFUNCTION(BlueprintCallable, Category="Interaction|Debug")
	void ToggleInteraction();

	UFUNCTION(BlueprintPure, Category="Interaction|Debug")
	bool IsInteractionEnabled() const { return bEnabled; }

public:
	// Scan configurations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Scan")
	float ScanInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Scan")
	float TraceDistance = 500.f;

	/** Optional sweep radius (<= 0 => line trace). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Scan")
	float TraceRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Scan")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Scan")
	bool bIgnoreOwner = true;

	// Hold configuration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction|Hold")
	float HoldTickInterval = 0.02f;

private:
	// Focus scanning
	void StartFocusScan();
	void StopFocusScan();
	void PerformFocusScan();

	bool FindInteractableInView(AActor*& OutActor, TScriptInterface<IInteractable>& OutInteractable) const;
	bool GetViewPoint(FVector& OutViewLoc, FRotator& OutViewRot) const;

	void SetFocused(AActor* NewActor, const TScriptInterface<IInteractable> NewInteractable);
	void ClearFocus();
	void RefreshQuery();

	// Press
	void ExecutePress();

	// Hold
	void BeginHold(float DurationSeconds);
	void TickHold();
	void CompleteHold();
	void ResetHold();

private:
	TWeakObjectPtr<AActor> InteractorActor;
	TWeakObjectPtr<AActor> FocusedActor;
	TScriptInterface<IInteractable> FocusedInteractable;
	
	FInteractionQueryResult CachedQueryResult;

	FTimerHandle FocusScanTimer;

	// Hold state
	bool bIsHolding = false;
	float HoldElapsed = 0.f;
	float HoldDuration = 0.f;
	FTimerHandle HoldTickTimer;
	
	bool bEnabled = true;
};
