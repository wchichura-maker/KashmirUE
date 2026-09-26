#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirMeleeDefenseProcessor.h"
#include "Misc/AutomationTest.h"


namespace
{
    FKashmirMeleeHitProcessResult MakeMeleeDefenseHit(
        const FVector& AttackDirection)
    {
        FKashmirMeleeHitProcessResult Result;

        Result.Evidence.InstigatorId =
            TEXT("Attacker");

        Result.Evidence.TargetId =
            TEXT("Defender");

        Result.Evidence.ContactSource =
            EKashmirContactSourceType::Weapon;

        Result.Evidence.SourceId =
            TEXT("Sword");

        Result.Evidence.ImpactPoint =
            FVector::ZeroVector;

        Result.Evidence.ImpactNormal =
            FVector::UpVector;

        Result.Evidence.AttackDirection =
            AttackDirection;

        Result.Evidence.ContactVelocity =
            AttackDirection.GetSafeNormal() *
            1000.0f;

        Result.Evidence.RelativeSpeed =
            1000.0f;
        FKashmirEffectResult DamageEffect;

        DamageEffect.Resolution =
            EKashmirResolutionType::Damage;

        DamageEffect.TargetId =
            TEXT("Defender");

        DamageEffect.EffectTag =
            FGameplayTag::RequestGameplayTag(
                TEXT("Effect.Damage")
            );

        DamageEffect.Magnitude =
            30.0f;

        Result.CombatResult.Effects.Add(
            DamageEffect
        );
        return Result;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMeleeDefenseProcessorMatrixTest,
    "Kashmir.Combat.MeleeDefense.Matrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)


bool FKashmirMeleeDefenseProcessorMatrixTest::RunTest(
    const FString& Parameters)
{
    struct FCase
    {
        const TCHAR* Name;

        FVector AttackDirection;

        bool bGuardActive;

        float GuardDamage;
        float Stamina;

        bool bExpectedBlocked;
        bool bExpectedGuardBreak;
        bool bExpectedDamageAllowed;
    };

    const TArray<FCase> Cases =
    {
        {
            TEXT("UnblockedFront"),
            FVector(-1.0f, 0.0f, 0.0f),
            false,
            30.0f,
            100.0f,
            false,
            false,
            true
        },

        {
            TEXT("BlockedFront"),
            FVector(-1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            100.0f,
            true,
            false,
            false
        },

        {
            TEXT("GuardBreakFront"),
            FVector(-1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            20.0f,
            true,
            true,
            false
        },

        {
            TEXT("SideAttack"),
            FVector(0.0f, -1.0f, 0.0f),
            true,
            30.0f,
            100.0f,
            false,
            false,
            true
        },

        {
            TEXT("RearAttack"),
            FVector(1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            100.0f,
            false,
            false,
            true
        }
    };

    FKashmirMeleeDefenseProcessor Processor;

    int32 SuccessfulCases = 0;

    for (const FCase& TestCase :
        Cases)
    {
        const FKashmirMeleeHitProcessResult HitResult =
            MakeMeleeDefenseHit(
                TestCase.AttackDirection
            );

        FKashmirMeleeDefenseInput Input;

        Input.DefenseInput.BlockState.bActive =
            TestCase.bGuardActive;

        Input.DefenseInput.BlockState.ForwardDirection =
            FVector::ForwardVector;

        Input.DefenseInput.BlockState.HalfAngleDegrees =
            60.0f;

        Input.DefenseInput.BaseGuardDamage =
            TestCase.GuardDamage;

        Input.DefenseInput.GuardDamageMultiplier =
            1.0f;

        Input.DefenseInput.AvailableStamina =
            TestCase.Stamina;

        FKashmirMeleeDefenseResult Result;
        FString Reason;

        const bool bResolved =
            Processor.Resolve(
                HitResult,
                Input,
                Result,
                Reason
            );

        const FString Prefix =
            FString::Printf(
                TEXT("[%s]"),
                TestCase.Name
            );

        const bool bCorrectBlocked =
            Result.DefenseResult.bBlocked ==
            TestCase.bExpectedBlocked;

        const bool bCorrectBreak =
            Result.DefenseResult.bGuardBroken ==
            TestCase.bExpectedGuardBreak;

        const bool bCorrectDamage =
            Result.bDamageAllowed ==
            TestCase.bExpectedDamageAllowed;
        bool bCorrectSuppression = false;
        bool bMagnitudePreserved = false;

        if (Result.HitResult.CombatResult.Effects.Num() == 1)
        {
            const FKashmirEffectResult& Effect =
                Result.HitResult.CombatResult.Effects[0];

            bCorrectSuppression =
                Effect.bSuppressed ==
                !TestCase.bExpectedDamageAllowed;

            bMagnitudePreserved =
                FMath::IsNearlyEqual(
                    Effect.Magnitude,
                    30.0f,
                    0.001f
                );
        }
        TestTrue(
            *FString::Printf(
                TEXT("%s effect suppression"),
                *Prefix
            ),
            bCorrectSuppression
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s original damage magnitude is preserved"),
                *Prefix
            ),
            bMagnitudePreserved
        );
        TestTrue(
            *FString::Printf(
                TEXT("%s resolves"),
                *Prefix
            ),
            bResolved
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s no error"),
                *Prefix
            ),
            Reason.IsEmpty()
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s block result"),
                *Prefix
            ),
            bCorrectBlocked
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s guard break result"),
                *Prefix
            ),
            bCorrectBreak
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s damage permission"),
                *Prefix
            ),
            bCorrectDamage
        );

        AddInfo(
            FString::Printf(
                TEXT(
                    "%s blocked=%s "
                    "guardBreak=%s "
                    "damageAllowed=%s"
                ),
                *Prefix,
                Result.DefenseResult.bBlocked
                    ? TEXT("true")
                    : TEXT("false"),
                Result.DefenseResult.bGuardBroken
                    ? TEXT("true")
                    : TEXT("false"),
                Result.bDamageAllowed
                    ? TEXT("true")
                    : TEXT("false")
            )
        );

        if (
            bResolved &&
            Reason.IsEmpty() &&
            bCorrectBlocked &&
            bCorrectBreak &&
            bCorrectDamage &&
            bCorrectSuppression &&
            bMagnitudePreserved)
        {
            ++SuccessfulCases;
        }
    }

    TestEqual(
        TEXT(
            "All melee defense cases pass"
        ),
        SuccessfulCases,
        Cases.Num()
    );

    return true;
}

#endif
