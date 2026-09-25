#include "Misc/AutomationTest.h"

#include "Combat/KashmirDamageResolver.h"

#if WITH_DEV_AUTOMATION_TESTS


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDamageResolverTorsoTest,
    "Kashmir.Combat.DamageResolver.Torso",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirDamageResolverTorsoTest::RunTest(
    const FString& Parameters)
{
    const FGameplayTag TorsoTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Torso")
        );

    FKashmirDamageInput Input;

    Input.BaseDamage =
        25.0f;

    Input.AttackPowerMultiplier =
        1.2f;

    Input.RegionMultiplier =
        1.0f;

    Input.HitRegion =
        TorsoTag;

    FKashmirDamageResult Result;
    FString Reason;

    FKashmirDamageResolver Resolver;

    const bool bResolved =
        Resolver.Resolve(
            Input,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Torso damage resolves"),
        bResolved
    );

    TestTrue(
        TEXT("Raw damage is thirty"),
        FMath::IsNearlyEqual(
            Result.RawDamage,
            30.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Torso final damage remains thirty"),
        FMath::IsNearlyEqual(
            Result.FinalDamage,
            30.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Torso region is preserved"),
        Result.HitRegion.MatchesTagExact(
            TorsoTag
        )
    );

    TestTrue(
        TEXT("Resolution succeeds without error"),
        Reason.IsEmpty()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDamageResolverRegionScalingTest,
    "Kashmir.Combat.DamageResolver.RegionScaling",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirDamageResolverRegionScalingTest::RunTest(
    const FString& Parameters)
{
    const FGameplayTag HeadTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Head")
        );

    FKashmirDamageInput Input;

    Input.BaseDamage =
        25.0f;

    Input.AttackPowerMultiplier =
        1.2f;

    Input.RegionMultiplier =
        1.5f;

    Input.HitRegion =
        HeadTag;

    FKashmirDamageResult Result;
    FString Reason;

    FKashmirDamageResolver Resolver;

    const bool bResolved =
        Resolver.Resolve(
            Input,
            Result,
            Reason
        );


    TestTrue(
        TEXT("Regional damage resolves"),
        bResolved
    );

    TestTrue(
        TEXT("Raw damage remains thirty"),
        FMath::IsNearlyEqual(
            Result.RawDamage,
            30.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Regional multiplier produces forty five"),
        FMath::IsNearlyEqual(
            Result.FinalDamage,
            45.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Head region is preserved"),
        Result.HitRegion.MatchesTagExact(
            HeadTag
        )
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDamageResolverRejectInvalidTest,
    "Kashmir.Combat.DamageResolver.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirDamageResolverRejectInvalidTest::RunTest(
    const FString& Parameters)
{
    FKashmirDamageInput Input;

    Input.BaseDamage =
        -10.0f;

    Input.AttackPowerMultiplier =
        1.0f;

    Input.RegionMultiplier =
        1.0f;

    Input.HitRegion =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Torso")
        );

    FKashmirDamageResult Result;
    FString Reason;

    FKashmirDamageResolver Resolver;

    const bool bResolved =
        Resolver.Resolve(
            Input,
            Result,
            Reason
        );

    TestFalse(
        TEXT("Negative damage is rejected"),
        bResolved
    );

    TestEqual(
        TEXT("Invalid damage explains reason"),
        Reason,
        FString(
            TEXT("base damage must be non-negative")
        )
    );

    return true;
}

#endif