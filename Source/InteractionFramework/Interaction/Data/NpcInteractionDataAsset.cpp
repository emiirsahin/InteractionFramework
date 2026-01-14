
#include "NpcInteractionDataAsset.h"

#if WITH_EDITOR

#include "Misc/DataValidation.h"

EDataValidationResult UNpcInteractionDataAsset::IsDataValid(FDataValidationContext& Context) const
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

	// Must have at least one state
	if (States.Num() == 0)
	{
		AddError(TEXT("States is empty. NPC interaction data must define at least one state."));
		return Result;
	}

	// Unique StateId + not None
	TSet<FName> SeenIds;
	for (int32 i = 0; i < States.Num(); ++i)
	{
		const FNpcDialogueState& State = States[i];

		if (State.StateId.IsNone())
		{
			AddError(FString::Printf(TEXT("States[%d] has StateId = None. Each state must have a unique StateId."), i));
		}
		else if (SeenIds.Contains(State.StateId))
		{
			AddError(FString::Printf(TEXT("Duplicate StateId '%s' found in States. StateId must be unique."), *State.StateId.ToString()));
		}
		else
		{
			SeenIds.Add(State.StateId);
		}

		// Validate requirements

		TSet<FName> SeenKeyIds;
		
		for (int32 r = 0; r < State.RequiredKeys.Num(); ++r)
		{
			const FInteractionKeyRequirement& Req = State.RequiredKeys[r];
			if (Req.KeyId.IsNone())
			{
				AddError(FString::Printf(TEXT("States[%d] RequiredKeys[%d] has KeyId=None. KeyId must be set."), i, r));
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
		}

		// If the state has requirements but no missing line.
		if (State.RequiredKeys.Num() > 0 && State.LineIfMissing.IsEmpty())
		{
			AddWarning(FString::Printf(TEXT("States[%d] has requirements but LineIfMissing is empty."), i));
		}

		// If state has no requirements and no met line.
		if (State.RequiredKeys.Num() == 0 && State.LineIfMet.IsEmpty())
		{
			AddWarning(FString::Printf(TEXT("States[%d] has no requirements and LineIfMet is empty."), i));
		}
	}

	// DefaultStateId must be set and exist
	if (DefaultStateId.IsNone())
	{
		AddError(TEXT("DefaultStateId is None. It must reference one of the States."));
	}
	else if (!FindStateById(DefaultStateId))
	{
		AddError(FString::Printf(TEXT("DefaultStateId '%s' does not match any StateId in States."), *DefaultStateId.ToString()));
	}
	
	if (PromptText.IsEmpty())
	{
		AddWarning(TEXT("PromptText is empty. The interaction widget will show no prompt (recommended: 'Talk')."));
	}

	return Result;
}

void UNpcInteractionDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (DefaultStateId.IsNone() || !FindStateById(DefaultStateId))
	{
		for (const FNpcDialogueState& State : States)
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
