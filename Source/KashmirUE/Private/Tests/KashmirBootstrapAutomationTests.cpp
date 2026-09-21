#if WITH_DEV_AUTOMATION_TESTS

#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirBootstrapSmokeTest,
    "Kashmir.Foundation.Bootstrap.Smoke",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FKashmirBootstrapSmokeTest::RunTest(const FString& Parameters)
{
    const FGameplayTag StaminaTag = UGameplayTagsManager::Get().RequestGameplayTag(
        FName(TEXT("Resource.Stamina")), false
    );
    const FGameplayTag ManaTag = UGameplayTagsManager::Get().RequestGameplayTag(
        FName(TEXT("Resource.Mana")), false
    );

    TestTrue(TEXT("The stamina gameplay tag is registered"), StaminaTag.IsValid());
    TestTrue(TEXT("The mana gameplay tag is registered"), ManaTag.IsValid());
    return true;
}

#endif
