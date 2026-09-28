#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDefensePipeline.h"
#include "Combat/KashmirPhysicalReactionResolver.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirHitEvidence MakePhysicalReactionEvidence()
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
        Evidence.HitRegion = FGameplayTag::RequestGameplayTag(TEXT("HitRegion.Torso"));
        return Evidence;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirPhysicalReactionResolverMatrixTest,
    "Kashmir.Combat.PhysicalReaction.ResolutionMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirPhysicalReactionResolverMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirPhysicalReactionResolver Resolver;
    FKashmirPhysicalReactionInput Input;
    Input.Evidence = MakePhysicalReactionEvidence();
    Input.Config.IntensityMultiplier = 2.0f;
    Input.Config.MaximumIntensity = 150.0f;
    Input.Config.PartialBodyBlendWeight = 0.40f;
    Input.Config.RecoveryDurationSeconds = 0.50f;
    Input.Config.RagdollCandidateThreshold = 150.0f;

    FKashmirPhysicalReactionResult Result;
    FString Reason;
    TestTrue(TEXT("No stagger resolves"), Resolver.Resolve(Input, Result, Reason));
    TestFalse(TEXT("No stagger requests no reaction"), Result.bReactionRequested);

    Input.Stagger.bStaggered = true;
    Input.Stagger.Target = EKashmirStaggerTarget::Defender;
    Input.Stagger.PhysicalIntensity = 100.0f;
    TestTrue(TEXT("Defender stagger resolves"), Resolver.Resolve(Input, Result, Reason));
    TestTrue(TEXT("Defender stagger requests reaction"), Result.bReactionRequested);
    TestEqual(TEXT("Default mode is partial body"), Result.Mode, EKashmirPhysicalReactionMode::PartialBody);
    TestTrue(TEXT("Defender follows attack travel"), Result.Direction.Equals(FVector(-1.0f, 0.0f, 0.0f)));
    TestEqual(TEXT("Reaction intensity is capped"), Result.Intensity, 150.0f);
    TestEqual(TEXT("Partial blend is preserved"), Result.PartialBodyBlendWeight, 0.40f);
    TestEqual(TEXT("Recovery duration is preserved"), Result.RecoveryDurationSeconds, 0.50f);
    TestEqual(TEXT("Defender keeps semantic region"), Result.HitRegion, Input.Evidence.HitRegion);

    Input.Stagger.Target = EKashmirStaggerTarget::Attacker;
    TestTrue(TEXT("Attacker stagger resolves"), Resolver.Resolve(Input, Result, Reason));
    TestTrue(TEXT("Attacker reacts toward attack origin"), Result.Direction.Equals(FVector::ForwardVector));
    TestFalse(TEXT("Attacker does not inherit defender hit region"), Result.HitRegion.IsValid());

    Input.Config.bAllowRagdollCandidate = true;
    TestTrue(TEXT("Ragdoll candidate resolves"), Resolver.Resolve(Input, Result, Reason));
    TestEqual(TEXT("Exact threshold becomes candidate"), Result.Mode, EKashmirPhysicalReactionMode::RagdollCandidate);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirPhysicalReactionResolverInvalidInputTest,
    "Kashmir.Combat.PhysicalReaction.RejectInvalidInput",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirPhysicalReactionResolverInvalidInputTest::RunTest(const FString& Parameters)
{
    const TArray<float> InvalidValues =
    {
        -1.0f,
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity()
    };

    FKashmirPhysicalReactionResolver Resolver;
    for (const float InvalidValue : InvalidValues)
    {
        FKashmirPhysicalReactionInput Input;
        Input.Evidence = MakePhysicalReactionEvidence();
        Input.Config.IntensityMultiplier = InvalidValue;
        FKashmirPhysicalReactionResult Result;
        FString Reason;
        TestFalse(TEXT("Invalid multiplier is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input.Config = {};
        Input.Config.MaximumIntensity = InvalidValue;
        TestFalse(TEXT("Invalid maximum is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input.Config = {};
        Input.Config.RecoveryDurationSeconds = InvalidValue;
        TestFalse(TEXT("Invalid recovery is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input.Config = {};
        Input.Config.RagdollCandidateThreshold = InvalidValue;
        TestFalse(TEXT("Invalid ragdoll threshold is rejected"), Resolver.Resolve(Input, Result, Reason));

        Input.Config = {};
        Input.Stagger.PhysicalIntensity = InvalidValue;
        TestFalse(TEXT("Invalid stagger intensity is rejected"), Resolver.Resolve(Input, Result, Reason));
    }

    FKashmirPhysicalReactionInput Input;
    Input.Evidence = MakePhysicalReactionEvidence();
    Input.Config.PartialBodyBlendWeight = 1.01f;
    FKashmirPhysicalReactionResult Result;
    FString Reason;
    TestFalse(TEXT("Blend above one is rejected"), Resolver.Resolve(Input, Result, Reason));

    Input.Config.PartialBodyBlendWeight = -0.01f;
    TestFalse(TEXT("Negative blend is rejected"), Resolver.Resolve(Input, Result, Reason));

    Input.Config = {};
    Input.Stagger.bStaggered = true;
    Input.Stagger.Target = EKashmirStaggerTarget::Defender;
    Input.Evidence.AttackDirection = FVector::ZeroVector;
    TestFalse(TEXT("Active reaction rejects zero direction"), Resolver.Resolve(Input, Result, Reason));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirPhysicalReactionPipelineIntegrationTest,
    "Kashmir.Combat.PhysicalReaction.DefensePipelineIntegration",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirPhysicalReactionPipelineIntegrationTest::RunTest(const FString& Parameters)
{
    FKashmirDefensePipeline Pipeline;
    const FKashmirHitEvidence Evidence = MakePhysicalReactionEvidence();
    FKashmirDefensePipelineInput Input;
    Input.BlockState.bActive = true;
    Input.BlockState.ForwardDirection = FVector::ForwardVector;
    Input.BlockState.HalfAngleDegrees = 60.0f;
    Input.BaseGuardDamage = 30.0f;
    Input.AvailableStamina = 20.0f;
    Input.PhysicalReactionConfig.IntensityMultiplier = 2.0f;
    Input.PhysicalReactionConfig.MaximumIntensity = 50.0f;

    FKashmirDefensePipelineResult Result;
    FString Reason;
    TestTrue(TEXT("Guard break reaction pipeline resolves"), Pipeline.Resolve(Evidence, Input, Result, Reason));
    TestTrue(TEXT("Guard break requests physical reaction"), Result.bPhysicalReactionRequested);
    TestEqual(TEXT("Guard break targets defender"), Result.PhysicalReaction.Target, EKashmirStaggerTarget::Defender);
    TestEqual(TEXT("Guard break intensity is capped"), Result.PhysicalReaction.Intensity, 50.0f);

    Input.AvailableStamina = 100.0f;
    TestTrue(TEXT("Regular block pipeline resolves"), Pipeline.Resolve(Evidence, Input, Result, Reason));
    TestFalse(TEXT("Regular block requests no reaction"), Result.bPhysicalReactionRequested);

    Input.ParryState.bActive = true;
    Input.ParryState.ElapsedTimeSeconds = 0.15f;
    Input.ParryState.Window.StartTimeSeconds = 0.10f;
    Input.ParryState.Window.EndTimeSeconds = 0.20f;
    Input.StaggerConfig.MinimumDeflectIntensity = 100.0f;
    TestTrue(TEXT("Deflect reaction pipeline resolves"), Pipeline.Resolve(Evidence, Input, Result, Reason));
    TestTrue(TEXT("Deflect requests physical reaction"), Result.bPhysicalReactionRequested);
    TestEqual(TEXT("Deflect targets attacker"), Result.PhysicalReaction.Target, EKashmirStaggerTarget::Attacker);
    TestTrue(TEXT("Deflect points toward origin"), Result.PhysicalReaction.Direction.Equals(FVector::ForwardVector));
    TestEqual(TEXT("Parry still preserves stamina"), Result.Guard.StaminaAfter, 100.0f);
    return true;
}

#endif
