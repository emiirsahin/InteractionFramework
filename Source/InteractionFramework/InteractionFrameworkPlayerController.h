// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InteractionFrameworkPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UInteractionComponent;
class UInteractionPromptWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class INTERACTIONFRAMEWORK_API AInteractionFrameworkPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AInteractionFrameworkPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
protected:

	/** Widget class (assign BP subclass in defaults). */
	UPROPERTY(EditDefaultsOnly, Category="Interaction|UI")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;
	
	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;
	
	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

private:
	
	UPROPERTY()
	UInteractionPromptWidget* PromptWidget = nullptr;

	UPROPERTY()
	UInteractionComponent* CachedInteractionComponent = nullptr;

	void CreatePromptWidgetIfNeeded();
	void BindToInteractionComponent(UInteractionComponent* InteractionComp);
	void UnbindFromInteractionComponent();

	// Callbacks from InteractionComponent
	UFUNCTION()
	void HandleFocusChanged(AActor* NewFocused, AActor* PrevFocused);

	UFUNCTION()
	void HandleQueryUpdated(const FInteractionQueryResult& Query);

	UFUNCTION()
	void HandleHoldProgress(float Progress);

	UFUNCTION()
	void HandleHoldReset();

	UFUNCTION()
	void HandleHoldCompleted();
};
