#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDefensePipeline.h"
#include "Combat/KashmirStaggerResolver.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirHitEvidence MakeStaggerEvidence()
    {
        FKashmirHitEvidence Evidence;
        Evidence.InstigatorId = TEXT("Attacker");
        Evidence.TargetId = TEXT("Defender");
        Evidence.ContactSource = EKashmirContactSourceType::Weapon;
        Evidence.SourceId = TEXT("Sword");
        Evidence.ImpactNormal = FVector::UpVector;
        Evidence.ContactVelocity = FVector(-120.0f, 0.0f, 0.0f);
        Evidence.AttackDirection = FVector(-1.0f, 0.0f, 0.0f);
        Evidence.RelativeSpeed = 120.0f;
        return Evidence;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirStaggerResolverMatrixTest,
    "Kashmir.Combat.Stagger.ResolutionMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirStaggerResolverMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirStaggerResolver Resolver;
    FKashmirStaggerInput Input;
    Input.Config.MinimumDeflectIntensity = 50.0f;
    Input.Config.DeflectDurationSeconds = 0.25f;
    Input.Config.GuardBreakDurationSeconds = 0.75f;

    FKashmirStaggerResult Result;
    FString Reason;

    TestTrue(TEXT("No reaction resolves"), Resolver.Resolve(Input, Result, Reason));
    TestFalse(TEXT("No reaction does not stagger"), Result.bStaggered);

    Input.Deflect.bDeflected = true;
    Input.Deflect.PhysicalIntensity = 50.0f;
    TestTrue(TEXT("Exact deflect threshold resolves"), Resolver.Resolve(Input, Result, Reason));
    TestTrue(TEXT("Exact threshold staggers"), Result.bStaggered);
    TestEqual(TEXT("Deflect staggers attacker"), Result.Target, EKashmirStaggerTarget::Attacker);
    TestEqual(TEXT("Deflect cause is explicit"), Result.Cause, EKashmirStaggerCause::Deflect);
    TestEqual(TEXT("Deflect duration is configured"), Result.DurationSeconds, 0.25f);

    Input.Deflect.PhysicalIntensity = 49.99f;
    TestTrue(TEXT("Below threshold resolves"), Resolver.Resolve(Input, Result, Reason));
    TestFalse(TEXT("Below threshold does not stagger"), Result.bStaggered);

    Input.Deflect = {};
    Input.Guard.bGuardBroken = true;
    Input.Guard.FinalGuardDamage = 40.0f;
    TestTrue(TEXT("Guard break resolves"), Resolver.Resolve(Input, Result, Reason));
    TestTrue(TEXT("Guard break staggers"), Result.bStaggered);
    TestEqual(TEXT("Guard break staggers defender"), Result.Target, EKashmirStaggerTarget::Defender);
    TestEqual(TEXT("Guard break cause is explicit"), Result.Cause, EKashmirStaggerCause::GuardBreak);
    TestEqual(TEXT("Guard break duration is configured"), Result.DurationSeconds, 0.75f);
    TestEqual(TEXT("Guard damage becomes physical cue"), Result.PhysicalIntensity, 40.0f);

    Input.Deflect.bDeflected = true;
    Input.Deflect.PhysicalIntensity = 100.0f;
    TestTrue(TEXT("Combined input resolves"), Resolver.Resolve(Input, Result, Reason));
    TestEqual(TEXT("Deflect preserves pipeline priority"), Result.Cause, EKashmirStaggerCause::Deflect);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirStaggerResolverInvalidInputTest,
    "Kashmir.Combat.Stagger.RejectInvalidInput",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirStaggerResolverInvalidInputTest::RunTest(const FString& Parameters)
{
    const TArray<float> InvalidValues =
    {
        -1.0f,
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity()
    };

    FKashmirStaggerResolver Resolver;
    for (const float InvalidValue : InvalidValues)
    {
        FKashmirStaggerInput Input;
        Input.Config.MinimumDeflectIntensity = InvalidValue;
        FKashmirStaggerResult Result;
        FString Reason;
        TestFalse(TEXT("Invalid minimum intensity is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input = {};
        Input.Config.DeflectDurationSeconds = InvalidValue;
        TestFalse(TEXT("Invalid deflect duration is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input = {};
        Input.Config.GuardBreakDurationSeconds = InvalidValue;
        TestFalse(TEXT("Invalid guard break duration is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input = {};
        Input.Deflect.PhysicalIntensity = InvalidValue;
        TestFalse(TEXT("Invalid physical intensity is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input = {};
        Input.Guard.FinalGuardDamage = InvalidValue;
        TestFalse(TEXT("Invalid guard damage is rejected"), Resolver.Resolve(Input, Result, Reason));
    }

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirStaggerDefensePipelineIntegrationTest,
    "Kashmir.Combat.Stagger.DefensePipelineIntegration",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirStaggerDefensePipelineIntegrationTest::RunTest(const FString& Parameters)
{
    FKashmirDefensePipeline Pipeline;
    const FKashmirHitEvidence Evidence = MakeStaggerEvidence();
    FKashmirDefensePipelineInput Input;
    Input.BlockState.bActive = true;
    Input.BlockState.ForwardDirection = FVector::ForwardVector;
    Input.BlockState.HalfAngleDegrees = 60.0f;
    Input.BaseGuardDamage = 30.0f;
    Input.AvailableStamina = 20.0f;
    Input.StaggerConfig.GuardBreakDurationSeconds = 0.80f;

    FKashmirDefensePipelineResult Result;
    FString Reason;
    TestTrue(TEXT("Guard break pipeline resolves"), Pipeline.Resolve(Evidence, Input, Result, Reason));
    TestTrue(TEXT("Guard break remains explicit"), Result.bGuardBroken);
    TestTrue(TEXT("Guard break produces stagger"), Result.bStaggered);
    TestEqual(TEXT("Defender receives guard break stagger"), Result.Stagger.Target, EKashmirStaggerTarget::Defender);
    TestEqual(TEXT("Guard break uses configured duration"), Result.Stagger.DurationSeconds, 0.80f);

    Input.AvailableStamina = 30.0f;
    TestTrue(TEXT("Exact stamina pipeline resolves"), Pipeline.Resolve(Evidence, Input, Result, Reason));
    TestFalse(TEXT("Exact stamina still does not break guard"), Result.bGuardBroken);
    TestFalse(TEXT("Exact stamina does not stagger"), Result.bStaggered);

    Input.ParryState.bActive = true;
    Input.ParryState.ElapsedTimeSeconds = 0.15f;
    Input.ParryState.Window.StartTimeSeconds = 0.10f;
    Input.ParryState.Window.EndTimeSeconds = 0.20f;
    Input.DeflectStrengthMultiplier = 1.0f;
    Input.StaggerConfig.MinimumDeflectIntensity = 100.0f;
    Input.StaggerConfig.DeflectDurationSeconds = 0.30f;
    TestTrue(TEXT("Deflect pipeline resolves"), Pipeline.Resolve(Evidence, Input, Result, Reason));
    TestTrue(TEXT("Parry produces deflect"), Result.bDeflected);
    TestTrue(TEXT("Deflect produces stagger"), Result.bStaggered);
    TestEqual(TEXT("Attacker receives deflect stagger"), Result.Stagger.Target, EKashmirStaggerTarget::Attacker);
    TestEqual(TEXT("Deflect uses configured duration"), Result.Stagger.DurationSeconds, 0.30f);
    TestEqual(TEXT("Parry preserves stamina"), Result.Guard.StaminaAfter, 30.0f);
    return true;
}

#endif
