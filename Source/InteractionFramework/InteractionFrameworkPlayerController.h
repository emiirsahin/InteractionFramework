// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InteractionFrameworkPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

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

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;
	
	/** Input mapping context setup */
	virtual void SetupInputComponent() override;
};
