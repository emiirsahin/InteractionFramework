#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interaction/Data/InteractionTypes.h"
#include "InteractionDebugHelper.generated.h"

class UInteractionComponent;

USTRUCT()
struct FInteractionDebugSnapshot
{
	GENERATED_BODY()

	UPROPERTY() TWeakObjectPtr<const AActor> Owner;
	UPROPERTY() TWeakObjectPtr<const AActor> FocusedActor;

	UPROPERTY() FVector TraceStart = FVector::ZeroVector;
	UPROPERTY() FVector TraceEnd   = FVector::ZeroVector;

	UPROPERTY() bool bTraceHit = false;
	UPROPERTY() TWeakObjectPtr<const AActor> HitActor;
	UPROPERTY() FHitResult HitResult;

	UPROPERTY() FInteractionQueryResult QueryResult;

	UPROPERTY() bool bEnabled = true;
	UPROPERTY() bool bHolding = false;
	UPROPERTY() float HoldProgress01 = 0.f;

	UPROPERTY() float ScanInterval = 0.f;
	UPROPERTY() float TraceDistance = 0.f;
	UPROPERTY() float TraceRadius = 0.f;
	UPROPERTY() bool bHitWasInteractable = false;
};

UCLASS()
class INTERACTIONFRAMEWORK_API UInteractionDebugHelper : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const UInteractionComponent* InOwnerComp);

	void SetEnabled(bool bInEnabled);
	bool IsEnabled() const { return bEnabled; }

	void SetDrawTrace(bool bInDraw) { bDrawTrace = bInDraw; }
	void SetDrawDuration(float InSeconds) { DrawDuration = InSeconds; }

	// Main entrypoint
	void Update(const FInteractionDebugSnapshot& Snapshot);

private:
	TWeakObjectPtr<const UInteractionComponent> OwnerComp;
	bool bEnabled = false;
	bool bDrawTrace = true;
	float DrawDuration = 0.06f;

	void Draw(const FInteractionDebugSnapshot& Snapshot) const;
	void Print(const FInteractionDebugSnapshot& Snapshot);
	FString BuildString(const FInteractionDebugSnapshot& Snapshot) const;

	uint64 GetMessageKey() const;

	double LastPrintTime = 0.0;

	UPROPERTY()
	float PrintInterval = 0.15f;
};
