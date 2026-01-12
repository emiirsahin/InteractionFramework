#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NpcSpeechBubbleWidget.generated.h"

UCLASS()
class INTERACTIONFRAMEWORK_API UNpcSpeechBubbleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category="NPC|UI")
	void SetLineText(const FText& Line);

	UFUNCTION(BlueprintImplementableEvent, Category="NPC|UI")
	void ClearLineText();
};
