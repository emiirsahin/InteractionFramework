// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionFrameworkPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Interaction/InteractionPromptWidget.h"
#include "Interaction/InteractionComponent.h"
#include "Blueprint/UserWidget.h"
#include "InteractionFrameworkCameraManager.h"

AInteractionFrameworkPlayerController::AInteractionFrameworkPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AInteractionFrameworkCameraManager::StaticClass();
}
void AInteractionFrameworkPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AInteractionFrameworkPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CreatePromptWidgetIfNeeded();

	// Bind if pawn already exists at BeginPlay.
	if (APawn* P = GetPawn())
	{
		OnPossess(P);
	}
}

void AInteractionFrameworkPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn) return;

	UInteractionComponent* InteractionComp = InPawn->FindComponentByClass<UInteractionComponent>();
	BindToInteractionComponent(InteractionComp);
}

void AInteractionFrameworkPlayerController::OnUnPossess()
{
	UnbindFromInteractionComponent();
	Super::OnUnPossess();
}

void AInteractionFrameworkPlayerController::CreatePromptWidgetIfNeeded()
{
	if (PromptWidget) return;
	if (!InteractionPromptWidgetClass) return;

	PromptWidget = CreateWidget<UInteractionPromptWidget>(this, InteractionPromptWidgetClass);
	if (!PromptWidget) return;

	PromptWidget->AddToViewport();
	
	PromptWidget->BP_SetPromptVisible(false);
	PromptWidget->BP_SetHoldProgress(0.f);
}

void AInteractionFrameworkPlayerController::BindToInteractionComponent(UInteractionComponent* InteractionComp)
{
	UnbindFromInteractionComponent();
	CachedInteractionComponent = InteractionComp;

	if (!CachedInteractionComponent || !PromptWidget)
	{
		return;
	}

	CachedInteractionComponent->OnFocusChanged.AddDynamic(this, &AInteractionFrameworkPlayerController::HandleFocusChanged);
	CachedInteractionComponent->OnQueryUpdated.AddDynamic(this, &AInteractionFrameworkPlayerController::HandleQueryUpdated);
	CachedInteractionComponent->OnHoldProgress.AddDynamic(this, &AInteractionFrameworkPlayerController::HandleHoldProgress);
	CachedInteractionComponent->OnHoldReset.AddDynamic(this, &AInteractionFrameworkPlayerController::HandleHoldReset);
	CachedInteractionComponent->OnHoldCompleted.AddDynamic(this, &AInteractionFrameworkPlayerController::HandleHoldCompleted);

	// Initialize UI with current state.
	HandleQueryUpdated(CachedInteractionComponent->GetCachedQueryResult());
}

void AInteractionFrameworkPlayerController::UnbindFromInteractionComponent()
{
	if (!CachedInteractionComponent) return;

	CachedInteractionComponent->OnFocusChanged.RemoveDynamic(this, &AInteractionFrameworkPlayerController::HandleFocusChanged);
	CachedInteractionComponent->OnQueryUpdated.RemoveDynamic(this, &AInteractionFrameworkPlayerController::HandleQueryUpdated);
	CachedInteractionComponent->OnHoldProgress.RemoveDynamic(this, &AInteractionFrameworkPlayerController::HandleHoldProgress);
	CachedInteractionComponent->OnHoldReset.RemoveDynamic(this, &AInteractionFrameworkPlayerController::HandleHoldReset);
	CachedInteractionComponent->OnHoldCompleted.RemoveDynamic(this, &AInteractionFrameworkPlayerController::HandleHoldCompleted);

	CachedInteractionComponent = nullptr;

	// Hide prompt when unpossessed
	if (PromptWidget)
	{
		PromptWidget->BP_SetPromptVisible(false);
		PromptWidget->BP_SetHoldProgress(0.f);
	}
}

void AInteractionFrameworkPlayerController::HandleFocusChanged(AActor* NewFocused, AActor* PrevFocused)
{
	// If the UI wants to do focus based updates, it can be done here.
	(void)NewFocused;
	(void)PrevFocused;
}

void AInteractionFrameworkPlayerController::HandleQueryUpdated(const FInteractionQueryResult& Query)
{
	if (!PromptWidget) return;

	const bool bVisible = Query.bShouldShowPrompt;
	PromptWidget->BP_SetPromptVisible(bVisible);

	if (!bVisible)
	{
		PromptWidget->BP_SetHoldProgress(0.f);
		return;
	}

	PromptWidget->BP_SetQueryResult(Query);

	// If press interaction, keep progress at 0. Because the progress bar exist in both interaction types. I
	// t is cosmetic for the press interaction.
	if (Query.InputType != EInteractionInputType::Hold)
	{
		PromptWidget->BP_SetHoldProgress(0.f);
	}
}

void AInteractionFrameworkPlayerController::HandleHoldProgress(float Progress)
{
	if (!PromptWidget) return;
	PromptWidget->BP_SetHoldProgress(Progress);
}

void AInteractionFrameworkPlayerController::HandleHoldReset()
{
	if (!PromptWidget) return;
	PromptWidget->BP_SetHoldProgress(0.f);
}

void AInteractionFrameworkPlayerController::HandleHoldCompleted()
{
	// When the hold interaction is complete, the UI can be updated through here.
}
