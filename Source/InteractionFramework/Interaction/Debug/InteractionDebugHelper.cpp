#include "InteractionDebugHelper.h"
#include "Interaction/InteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

void UInteractionDebugHelper::Initialize(const UInteractionComponent* InOwnerComp)
{
	OwnerComp = InOwnerComp;
}

void UInteractionDebugHelper::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;

	// Clear overlay when disabling
	if (!bEnabled && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(GetMessageKey(), 0.f, FColor::White, TEXT(""));
	}
}

void UInteractionDebugHelper::Update(const FInteractionDebugSnapshot& Snapshot)
{
	if (!bEnabled) return;

	Draw(Snapshot);
	Print(Snapshot);
}

uint64 UInteractionDebugHelper::GetMessageKey() const
{
	// Onscreen message overwrite key
	return (uint64)reinterpret_cast<uintptr_t>(this);
}

void UInteractionDebugHelper::Draw(const FInteractionDebugSnapshot& S) const
{
	if (!bDrawTrace) return;

	const UInteractionComponent* Comp = OwnerComp.Get();
	UWorld* World = Comp ? Comp->GetWorld() : nullptr;
	if (!World) return;

	DrawDebugLine(World, S.TraceStart, S.TraceEnd, FColor::Cyan, false, DrawDuration, 0, 1.0f);

	if (S.TraceRadius > 0.f)
	{
		DrawDebugSphere(World, S.TraceEnd, S.TraceRadius, 12, FColor::Cyan, false, DrawDuration);
	}

	if (S.bTraceHit)
	{
		DrawDebugPoint(World, S.HitResult.ImpactPoint, 12.f, FColor::Green, false, DrawDuration);
		DrawDebugLine(World,
			S.HitResult.ImpactPoint,
			S.HitResult.ImpactPoint + S.HitResult.ImpactNormal * 20.f,
			FColor::Green, false, DrawDuration, 0, 1.0f);
	}
}

void UInteractionDebugHelper::Print(const FInteractionDebugSnapshot& S)
{
	if (!GEngine) return;

	const UInteractionComponent* Comp = OwnerComp.Get();
	UWorld* World = Comp ? Comp->GetWorld() : nullptr;
	if (!World) return;

	const double Now = World->GetTimeSeconds();
	if (Now - LastPrintTime < PrintInterval)
	{
		return;
	}

	LastPrintTime = Now;

	GEngine->AddOnScreenDebugMessage(GetMessageKey(), .2f, FColor::White, BuildString(S));
}


FString UInteractionDebugHelper::BuildString(const FInteractionDebugSnapshot& S) const
{
    const FString OwnerName = S.Owner.IsValid() ? S.Owner->GetName() : TEXT("None");
    const FString FocusName = S.FocusedActor.IsValid() ? S.FocusedActor->GetName() : TEXT("None");
    const FString HitName   = S.HitActor.IsValid() ? S.HitActor->GetName() : TEXT("None");

    const FString InputTypeStr =
        (S.QueryResult.InputType == EInteractionInputType::Hold) ? TEXT("Hold") : TEXT("Press");

    const FString PromptStr = S.QueryResult.PromptText.IsEmpty()
        ? TEXT("<empty>")
        : S.QueryResult.PromptText.ToString();

    const bool bAvailable = S.QueryResult.IsAvailable();
    const int32 UnmetCount = S.QueryResult.UnmetRequirementNumber;

    // Show up to 3 unmet messages to avoid overlay crowd
    FString UnmetPreview;
    const int32 MaxToShow = 3;
	if (S.QueryResult.UnmetRequirementMessages.Num() > 0)
	{
		for (int32 i = 0; i < FMath::Min(UnmetCount, MaxToShow); ++i)
		{
			if (i > 0) UnmetPreview += TEXT(" | ");
			UnmetPreview += S.QueryResult.UnmetRequirementMessages[i].ToString();
		}
		if (UnmetCount > MaxToShow)
		{
			UnmetPreview += FString::Printf(TEXT(" | ...(+%d)"), UnmetCount - MaxToShow);
		}
		if (UnmetCount == 0)
		{
			UnmetPreview = TEXT("<none>");
		}
	}
	
    return FString::Printf(
        TEXT("[Interaction Debug]\n")
        TEXT("Owner: %s | Enabled: %s\n")
        TEXT("Focused: %s\n")
        TEXT("TraceHit: %s | HitActor: %s | HitInteractable: %s\n")
        TEXT("PromptVisible: %s | Input: %s | HoldDuration: %.2f\n")
        TEXT("Prompt: %s\n")
        TEXT("Available: %s | Unmet: %d\n")
        TEXT("UnmetMessages: %s\n")
        TEXT("Holding: %s | HoldProgress: %.2f\n")
        TEXT("Scan: Interval=%.2f Dist=%.0f Radius=%.0f\n"),
        *OwnerName,
        S.bEnabled ? TEXT("Yes") : TEXT("No"),
        *FocusName,
        S.bTraceHit ? TEXT("Yes") : TEXT("No"),
        *HitName,
        S.bHitWasInteractable ? TEXT("Yes") : TEXT("No"),
        S.QueryResult.bShouldShowPrompt ? TEXT("Yes") : TEXT("No"),
        *InputTypeStr,
        S.QueryResult.HoldDuration,
        *PromptStr,
        bAvailable ? TEXT("Yes") : TEXT("No"),
        UnmetCount,
        *UnmetPreview,
        S.bHolding ? TEXT("Yes") : TEXT("No"),
        S.HoldProgress01,
        S.ScanInterval,
        S.TraceDistance,
        S.TraceRadius
    );
}

