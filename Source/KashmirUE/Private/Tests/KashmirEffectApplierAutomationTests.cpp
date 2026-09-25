#include "Misc/AutomationTest.h"

#include "Combat/KashmirEffectApplier.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirEffectApplierDamageTest,
    "Kashmir.Combat.EffectApplication.Damage",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirEffectApplierDamageTest::RunTest(
    const FString& Parameters)
{
    FKashmirHealthState Health;

    Health.Current = 100.0f;
    Health.Maximum = 100.0f;

    FKashmirEffectResult Effect;

    Effect.Resolution =
        EKashmirResolutionType::Damage;

    Effect.TargetId =
        TEXT("Enemy_01");

    Effect.EffectTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Effect.Damage")
        );

    Effect.Magnitude =
        30.0f;

    FKashmirEffectApplicationResult Result;
    FString Reason;

    FKashmirEffectApplier Applier;

    const bool bApplied =
        Applier.Apply(
            Effect,
            Health,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Damage effect applies"),
        bApplied
    );

    TestTrue(
        TEXT("Health decreases from one hundred to seventy"),
        FMath::IsNearlyEqual(
            Health.Current,
            70.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Application records previous health"),
        FMath::IsNearlyEqual(
            Result.HealthBefore,
            100.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Application records resulting health"),
        FMath::IsNearlyEqual(
            Result.HealthAfter,
            70.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Thirty damage is actually applied"),
        FMath::IsNearlyEqual(
            Result.AppliedMagnitude,
            30.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Damage succeeds without error"),
        Reason.IsEmpty()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirEffectApplierHealTest,
    "Kashmir.Combat.EffectApplication.Heal",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirEffectApplierHealTest::RunTest(
    const FString& Parameters)
{
    FKashmirHealthState Health;

    Health.Current = 60.0f;
    Health.Maximum = 100.0f;

    FKashmirEffectResult Effect;

    Effect.Resolution =
        EKashmirResolutionType::Heal;

    Effect.TargetId =
        TEXT("Player");

    Effect.EffectTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Effect.Heal")
        );

    Effect.Magnitude =
        25.0f;

    FKashmirEffectApplicationResult Result;
    FString Reason;

    FKashmirEffectApplier Applier;

    const bool bApplied =
        Applier.Apply(
            Effect,
            Health,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Heal effect applies"),
        bApplied
    );

    TestTrue(
        TEXT("Health increases to eighty five"),
        FMath::IsNearlyEqual(
            Health.Current,
            85.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT("Twenty five healing is actually applied"),
        FMath::IsNearlyEqual(
            Result.AppliedMagnitude,
            25.0f,
            0.001f
        )
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirEffectApplierClampTest,
    "Kashmir.Combat.EffectApplication.Clamp",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirEffectApplierClampTest::RunTest(
    const FString& Parameters)
{
    FKashmirHealthState Health;

    Health.Current = 10.0f;
    Health.Maximum = 100.0f;

    FKashmirEffectResult Effect;

    Effect.Resolution =
        EKashmirResolutionType::Damage;

    Effect.Magnitude =
        30.0f;

    FKashmirEffectApplicationResult Result;
    FString Reason;

    FKashmirEffectApplier Applier;

    const bool bApplied =
        Applier.Apply(
            Effect,
            Health,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Lethal damage applies"),
        bApplied
    );

    TestTrue(
        TEXT("Health clamps at zero"),
        FMath::IsNearlyZero(
            Health.Current,
            0.001f
        )
    );

    TestTrue(
        TEXT("Only remaining ten health is applied"),
        FMath::IsNearlyEqual(
            Result.AppliedMagnitude,
            10.0f,
            0.001f
        )
    );

    return true;
}

#endif