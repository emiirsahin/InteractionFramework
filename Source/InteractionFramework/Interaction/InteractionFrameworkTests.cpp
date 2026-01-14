#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "KeyringComponent.h"
#include "InteractionDataAsset.h"
#include "InteractionUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeyring_AddRemove,
	"InteractionFramework.Keyring.AddRemove",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeyring_AddRemove::RunTest(const FString& Parameters)
{
	UKeyringComponent* Keyring = NewObject<UKeyringComponent>(GetTransientPackage());

	TestFalse(TEXT("HasKey(None) should be false"), Keyring->HasKey(NAME_None));
	TestTrue(TEXT("AddKey(TestKey) should succeed"), Keyring->AddKey("TestKey"));
	TestTrue(TEXT("HasKey(TestKey) should be true"), Keyring->HasKey("TestKey"));
	TestTrue(TEXT("RemoveKey(TestKey) should succeed"), Keyring->RemoveKey("TestKey"));
	TestFalse(TEXT("HasKey(TestKey) should now be false"), Keyring->HasKey("TestKey"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionDataAsset_DefaultState,
	"InteractionFramework.DataAsset.DefaultStateId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionDataAsset_DefaultState::RunTest(const FString& Parameters)
{
	UInteractionDataAsset* DA = NewObject<UInteractionDataAsset>(GetTransientPackage());

	FInteractionStateDefinition A;
	A.StateId = "A";

	FInteractionStateDefinition B;
	B.StateId = "B";

	DA->States = { A, B };

	// If DefaultStateId is None fall back to first entry
	DA->DefaultStateId = NAME_None;
	TestEqual(TEXT("Default when none should be first state"), DA->GetDefaultStateId(), FName("A"));

	// If DefaultStateId is set and exists use it
	DA->DefaultStateId = "B";
	TestEqual(TEXT("DefaultStateId set to B should return B"), DA->GetDefaultStateId(), FName("B"));

	//  Fallback to first if DefaultStateId is set but doesn't exist.
	DA->DefaultStateId = "DoesNotExist";
	TestEqual(TEXT("Invalid DefaultStateId should fall back to first"), DA->GetDefaultStateId(), FName("A"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionDataAsset_PromptOverride,
	"InteractionFramework.DataAsset.PromptOverridePolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInteractionDataAsset_PromptOverride::RunTest(const FString& Parameters)
{
	UInteractionDataAsset* DA = NewObject<UInteractionDataAsset>(GetTransientPackage());

	FInteractionStateDefinition State;
	State.StateId = "S";
	State.bShouldShowPrompt = false;

	DA->PromptOverridePolicy = EInteractionPromptOverride::UsePerState;
	TestFalse(TEXT("UsePerState should respect state false"), DA->ShouldShowPromptForState(State));

	DA->PromptOverridePolicy = EInteractionPromptOverride::ForceShow;
	TestTrue(TEXT("ForceShow should always show"), DA->ShouldShowPromptForState(State));

	DA->PromptOverridePolicy = EInteractionPromptOverride::ForceHide;
	TestFalse(TEXT("ForceHide should always hide"), DA->ShouldShowPromptForState(State));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRequirements_MissingMessages,
	"InteractionFramework.Requirements.MissingMessages",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRequirements_MissingMessages::RunTest(const FString& Parameters)
{
	UKeyringComponent* Keyring = NewObject<UKeyringComponent>(GetTransientPackage());
	Keyring->AddKey("RedKey");

	FInteractionKeyRequirement ReqRed;
	ReqRed.KeyId = "RedKey";
	ReqRed.MissingMessage = FText::FromString("Missing Red");

	FInteractionKeyRequirement ReqBlue;
	ReqBlue.KeyId = "BlueKey";
	ReqBlue.MissingMessage = FText::FromString("Missing Blue");

	TArray<FInteractionKeyRequirement> Requirements = { ReqRed, ReqBlue };

	TArray<FText> Missing;
	const bool bHasMissing = InteractionUtils::BuildMissingMessages(Requirements, Keyring, Missing);

	TestTrue(TEXT("Should have missing requirements (BlueKey missing)"), bHasMissing);
	TestEqual(TEXT("Should have exactly 1 missing message"), Missing.Num(), 1);
	TestEqual(TEXT("Missing message should be Blue"), Missing[0].ToString(), FString("Missing Blue"));

	return true;
}

#endif
