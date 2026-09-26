#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDefensePipeline.h"
#include "Combat/KashmirParryResolver.h"
#include "Misc/AutomationTest.h"


namespace
{
    FKashmirHitEvidence MakeParryEvidence()
    {
        FKashmirHitEvidence Evidence;
        Evidence.InstigatorId = TEXT("Attacker");
        Evidence.TargetId = TEXT("Defender");
        Evidence.ContactSource = EKashmirContactSourceType::Weapon;
        Evidence.SourceId = TEXT("Sword");
        Evidence.ImpactNormal = FVector::UpVector;
        Evidence.ContactVelocity = FVector(-1000.0f, 0.0f, 0.0f);
        Evidence.AttackDirection = FVector(-1.0f, 0.0f, 0.0f);
        Evidence.RelativeSpeed = 1000.0f;
        return Evidence;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirParryResolverTimingMatrixTest,
    "Kashmir.Combat.Parry.TimingMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirParryResolverTimingMatrixTest::RunTest(const FString& Parameters)
{
    struct FCase
    {
        const TCHAR* Name;
        bool bActive;
        float Elapsed;
        EKashmirParryTiming ExpectedTiming;
        bool bExpectedParried;
    };

    const TArray<FCase> Cases =
    {
        { TEXT("TooEarly"), true, 0.09f, EKashmirParryTiming::TooEarly, false },
        { TEXT("ExactStart"), true, 0.10f, EKashmirParryTiming::ExactStart, true },
        { TEXT("InsideWindow"), true, 0.15f, EKashmirParryTiming::InsideWindow, true },
        { TEXT("ExactEnd"), true, 0.20f, EKashmirParryTiming::ExactEnd, true },
        { TEXT("TooLate"), true, 0.21f, EKashmirParryTiming::TooLate, false },
        { TEXT("Inactive"), false, 0.15f, EKashmirParryTiming::Inactive, false }
    };

    FKashmirParryResolver Resolver;
    FKashmirBlockState Facing;
    Facing.ForwardDirection = FVector::ForwardVector;
    Facing.HalfAngleDegrees = 60.0f;

    for (const FCase& TestCase : Cases)
    {
        FKashmirParryState State;
        State.bActive = TestCase.bActive;
        State.ElapsedTimeSeconds = TestCase.Elapsed;
        State.Window.StartTimeSeconds = 0.10f;
        State.Window.EndTimeSeconds = 0.20f;

        FKashmirParryResult Result;
        FString Reason;
        const bool bResolved = Resolver.Resolve(
            MakeParryEvidence(), Facing, State, Result, Reason);

        TestTrue(*FString::Printf(TEXT("[%s] resolves"), TestCase.Name), bResolved);
        TestTrue(*FString::Printf(TEXT("[%s] no error"), TestCase.Name), Reason.IsEmpty());
        TestEqual(*FString::Printf(TEXT("[%s] timing"), TestCase.Name), Result.Timing, TestCase.ExpectedTiming);
        TestEqual(*FString::Printf(TEXT("[%s] parried"), TestCase.Name), Result.bParried, TestCase.bExpectedParried);
    }

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirParryResolverInvalidWindowTest,
    "Kashmir.Combat.Parry.InvalidWindow",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirParryResolverInvalidWindowTest::RunTest(const FString& Parameters)
{
    FKashmirParryState State;
    State.bActive = true;
    State.ElapsedTimeSeconds = 0.15f;
    State.Window.StartTimeSeconds = 0.20f;
    State.Window.EndTimeSeconds = 0.10f;

    FKashmirBlockState Facing;
    FKashmirParryResult Result;
    FString Reason;
    FKashmirParryResolver Resolver;

    TestFalse(
        TEXT("Invalid window is rejected"),
        Resolver.Resolve(MakeParryEvidence(), Facing, State, Result, Reason));
    TestFalse(TEXT("Invalid window provides a reason"), Reason.IsEmpty());
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirParryDefensePipelinePriorityTest,
    "Kashmir.Combat.Parry.DefensePipelinePriority",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirParryDefensePipelinePriorityTest::RunTest(const FString& Parameters)
{
    FKashmirDefensePipelineInput Input;
    Input.BlockState.bActive = true;
    Input.BlockState.ForwardDirection = FVector::ForwardVector;
    Input.BlockState.HalfAngleDegrees = 60.0f;
    Input.ParryState.bActive = true;
    Input.ParryState.ElapsedTimeSeconds = 0.15f;
    Input.ParryState.Window.StartTimeSeconds = 0.10f;
    Input.ParryState.Window.EndTimeSeconds = 0.20f;
    Input.BaseGuardDamage = 30.0f;
    Input.AvailableStamina = 20.0f;

    FKashmirDefensePipelineResult Result;
    FString Reason;
    FKashmirDefensePipeline Pipeline;

    TestTrue(TEXT("Pipeline resolves parry"), Pipeline.Resolve(MakeParryEvidence(), Input, Result, Reason));
    TestTrue(TEXT("Parry wins priority"), Result.bParried);
    TestFalse(TEXT("Parry does not also block"), Result.bBlocked);
    TestFalse(TEXT("Parry does not break guard"), Result.bGuardBroken);
    TestFalse(TEXT("Parry skips guard processing"), Result.Guard.bGuardProcessed);
    TestEqual(TEXT("Parry preserves stamina"), Result.Guard.StaminaAfter, 20.0f);
    return true;
}

#endif
