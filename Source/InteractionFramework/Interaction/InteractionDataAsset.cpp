
#if WITH_EDITOR

#include "InteractionDataAsset.h"
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
		if (Result == EDataValidationResult::Valid)
		{
			Result = EDataValidationResult::Valid;
		}
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

		if (State.bShouldShowPrompt && State.PromptText.IsEmpty())
		{
			AddWarning(FString::Printf(TEXT("State '%s' has bShouldShowPrompt=true but PromptText is empty."), *State.StateId.ToString()));
		}

		if (State.InputType == EInteractionInputType::Hold && State.HoldDuration <= 0.f)
		{
			AddWarning(FString::Printf(TEXT("State '%s' is Hold but HoldDuration <= 0."), *State.StateId.ToString()));
		}

		for (int32 r = 0; r < State.RequiredKeys.Num(); ++r)
		{
			const FInteractionKeyRequirement& Req = State.RequiredKeys[r];
			if (Req.KeyId.IsNone())
			{
				AddError(FString::Printf(TEXT("State '%s' RequiredKeys[%d] has KeyId == None."), *State.StateId.ToString(), r));
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

	// If DefaultStateId is None and we have states, set it to the first state id.
	if (DefaultStateId.IsNone() && States.Num() > 0 && !States[0].StateId.IsNone())
	{
		DefaultStateId = States[0].StateId;
	}
}

#endif
