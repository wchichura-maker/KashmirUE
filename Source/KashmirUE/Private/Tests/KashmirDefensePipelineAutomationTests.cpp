#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDefensePipeline.h"
#include "Misc/AutomationTest.h"


namespace
{
    FKashmirHitEvidence MakePipelineEvidence(
        const FVector& AttackDirection)
    {
        FKashmirHitEvidence Evidence;

        Evidence.InstigatorId =
            TEXT("Attacker");

        Evidence.TargetId =
            TEXT("Defender");

        Evidence.ContactSource =
            EKashmirContactSourceType::Weapon;

        Evidence.SourceId =
            TEXT("Sword");

        Evidence.ImpactPoint =
            FVector::ZeroVector;

        Evidence.ImpactNormal =
            FVector::UpVector;

        Evidence.ContactVelocity =
            AttackDirection.GetSafeNormal() *
            1000.0f;

        Evidence.AttackDirection =
            AttackDirection;

        Evidence.RelativeSpeed =
            1000.0f;

        return Evidence;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDefensePipelineMatrixTest,
    "Kashmir.Combat.DefensePipeline.Matrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)


bool FKashmirDefensePipelineMatrixTest::RunTest(
    const FString& Parameters)
{
    struct FPipelineCase
    {
        const TCHAR* Name;

        FVector AttackDirection;

        bool bGuardActive;

        float BaseGuardDamage;
        float AvailableStamina;

        bool bExpectedBlocked;
        bool bExpectedGuardBroken;

        float ExpectedStaminaAfter;
    };

    const TArray<FPipelineCase> Cases =
    {
        {
            TEXT("FrontEnoughStamina"),
            FVector(-1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            100.0f,
            true,
            false,
            70.0f
        },

        {
            TEXT("FrontExactStamina"),
            FVector(-1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            30.0f,
            true,
            false,
            0.0f
        },

        {
            TEXT("FrontGuardBreak"),
            FVector(-1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            20.0f,
            true,
            true,
            0.0f
        },

        {
            TEXT("SideAttack"),
            FVector(0.0f, -1.0f, 0.0f),
            true,
            30.0f,
            100.0f,
            false,
            false,
            100.0f
        },

        {
            TEXT("RearAttack"),
            FVector(1.0f, 0.0f, 0.0f),
            true,
            30.0f,
            100.0f,
            false,
            false,
            100.0f
        },

        {
            TEXT("FrontGuardInactive"),
            FVector(-1.0f, 0.0f, 0.0f),
            false,
            30.0f,
            100.0f,
            false,
            false,
            100.0f
        }
    };

    FKashmirDefensePipeline Pipeline;

    int32 SuccessfulCases = 0;

    for (const FPipelineCase& TestCase :
        Cases)
    {
        const FKashmirHitEvidence Evidence =
            MakePipelineEvidence(
                TestCase.AttackDirection
            );

        FKashmirDefensePipelineInput Input;

        Input.BlockState.bActive =
            TestCase.bGuardActive;

        Input.BlockState.ForwardDirection =
            FVector::ForwardVector;

        Input.BlockState.HalfAngleDegrees =
            60.0f;

        Input.BaseGuardDamage =
            TestCase.BaseGuardDamage;

        Input.GuardDamageMultiplier =
            1.0f;

        Input.AvailableStamina =
            TestCase.AvailableStamina;

        FKashmirDefensePipelineResult Result;
        FString Reason;

        const bool bResolved =
            Pipeline.Resolve(
                Evidence,
                Input,
                Result,
                Reason
            );

        const FString Prefix =
            FString::Printf(
                TEXT("[%s]"),
                TestCase.Name
            );

        TestTrue(
            *FString::Printf(
                TEXT("%s pipeline resolves"),
                *Prefix
            ),
            bResolved
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s produces no error"),
                *Prefix
            ),
            Reason.IsEmpty()
        );

        const bool bCorrectBlocked =
            Result.bBlocked ==
            TestCase.bExpectedBlocked;

        const bool bCorrectBreak =
            Result.bGuardBroken ==
            TestCase.bExpectedGuardBroken;

        const bool bCorrectStamina =
            FMath::IsNearlyEqual(
                Result.Guard.StaminaAfter,
                TestCase.ExpectedStaminaAfter,
                0.001f
            );

        TestTrue(
            *FString::Printf(
                TEXT("%s block result is correct"),
                *Prefix
            ),
            bCorrectBlocked
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s guard break result is correct"),
                *Prefix
            ),
            bCorrectBreak
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s stamina result is correct"),
                *Prefix
            ),
            bCorrectStamina
        );

        AddInfo(
            FString::Printf(
                TEXT(
                    "%s blocked=%s "
                    "guardBroken=%s "
                    "staminaBefore=%.2f "
                    "staminaAfter=%.2f"
                ),
                *Prefix,
                Result.bBlocked
                    ? TEXT("true")
                    : TEXT("false"),
                Result.bGuardBroken
                    ? TEXT("true")
                    : TEXT("false"),
                Result.Guard.StaminaBefore,
                Result.Guard.StaminaAfter
            )
        );

        if (
            bResolved &&
            Reason.IsEmpty() &&
            bCorrectBlocked &&
            bCorrectBreak &&
            bCorrectStamina)
        {
            ++SuccessfulCases;
        }
    }

    TestEqual(
        TEXT(
            "All defense pipeline cases pass"
        ),
        SuccessfulCases,
        Cases.Num()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDefensePipelineInvalidEvidenceTest,
    "Kashmir.Combat.DefensePipeline.RejectInvalidEvidence",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirDefensePipelineInvalidEvidenceTest::RunTest(
    const FString& Parameters)
{
    FKashmirHitEvidence Evidence =
        MakePipelineEvidence(
            FVector::ZeroVector
        );

    FKashmirDefensePipelineInput Input;

    Input.BlockState.bActive =
        true;

    Input.BlockState.ForwardDirection =
        FVector::ForwardVector;

    Input.BaseGuardDamage =
        20.0f;

    Input.AvailableStamina =
        100.0f;

    FKashmirDefensePipelineResult Result;
    FString Reason;

    FKashmirDefensePipeline Pipeline;

    const bool bResolved =
        Pipeline.Resolve(
            Evidence,
            Input,
            Result,
            Reason
        );

    TestFalse(
        TEXT(
            "Invalid evidence is rejected by pipeline"
        ),
        bResolved
    );

    TestFalse(
        TEXT(
            "Pipeline rejection contains reason"
        ),
        Reason.IsEmpty()
    );

    return true;
}

#endif