
#include "InteractionDataAsset.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

EDataValidationResult UInteractionDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	auto AddError = [&](const FString& Msg)
	{
		Context.AddError(FText::FromString(Msg));
		Result = EDataValidationResult::Invalid;
	};

	auto AddWarning = [&](const FString& Msg)
	{
		Context.AddWarning(FText::FromString(Msg));
	};

	if (States.Num() == 0)
	{
		AddError(TEXT("States is empty. InteractionDataAsset must define at least one state."));
		return Result;
	}

	// Check state IDs and duplicates
	TSet<FName> Seen;
	for (int32 i = 0; i < States.Num(); ++i)
	{
		const FInteractionStateDefinition& State = States[i];

		if (State.StateId.IsNone())
		{
			AddError(FString::Printf(TEXT("States[%d] has StateId == None."), i));
			continue;
		}

		if (Seen.Contains(State.StateId))
		{
			AddError(FString::Printf(TEXT("Duplicate StateId '%s' found in States."), *State.StateId.ToString()));
		}
		else
		{
			Seen.Add(State.StateId);
		}

		if (PromptOverridePolicy == EInteractionPromptOverride::ForceShow && State.PromptText.IsEmpty() || PromptOverridePolicy == EInteractionPromptOverride::UsePerState && State.bShouldShowPrompt && State.PromptText.IsEmpty())
		{
			AddWarning(FString::Printf(TEXT("State '%s' must show prompt but PromptText is empty."), *State.StateId.ToString()));
		}

		if (State.InputType == EInteractionInputType::Hold && State.HoldDuration <= 0.f)
		{
			AddWarning(FString::Printf(TEXT("State '%s' is Hold but HoldDuration <= 0."), *State.StateId.ToString()));
		}
		
		TSet<FName> SeenKeyIds;
		for (int32 r = 0; r < State.RequiredKeys.Num(); ++r)
		{
			const FInteractionKeyRequirement& Req = State.RequiredKeys[r];
			
			if (Req.KeyId.IsNone())
			{
				AddError(FString::Printf(TEXT("State '%s' RequiredKeys[%d] has KeyId == None."), *State.StateId.ToString(), r));
				continue;
			}
	
			if (SeenKeyIds.Contains(Req.KeyId))
			{
				AddError(FString::Printf(
					TEXT("States[%d] has duplicate RequiredKeys with KeyId '%s'. Each KeyId may only appear once per state."),
					i, *Req.KeyId.ToString()));
			}
			else
			{
				SeenKeyIds.Add(Req.KeyId);
			}
			
			if (Req.MissingMessage.IsEmpty())
			{
				AddWarning(FString::Printf(TEXT("State '%s' RequiredKeys[%d] MissingMessage is empty."), *State.StateId.ToString(), r));
			}
		}
	}

	// DefaultStateId must exist (or be None, but None state ID is corrected by PostEditChangeProperty)
	if (!DefaultStateId.IsNone())
	{
		bool bFoundDefault = false;
		for (const FInteractionStateDefinition& State : States)
		{
			if (State.StateId == DefaultStateId)
			{
				bFoundDefault = true;
				break;
			}
		}
		if (!bFoundDefault)
		{
			AddError(FString::Printf(TEXT("DefaultStateId '%s' does not exist in States."), *DefaultStateId.ToString()));
		}
	}

	return Result;
}

void UInteractionDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (DefaultStateId.IsNone() || !FindStateById(DefaultStateId))
	{
		for (const FInteractionStateDefinition& State : States)
		{
			if (!State.StateId.IsNone())
			{
				DefaultStateId = State.StateId;
				break;
			}
		}
	}
}

#endif
