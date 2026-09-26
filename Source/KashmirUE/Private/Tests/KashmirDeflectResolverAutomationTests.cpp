#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDefensePipeline.h"
#include "Combat/KashmirDeflectResolver.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirHitEvidence MakeDeflectEvidence(const FVector& AttackDirection = FVector(-1.0f, 0.0f, 0.0f))
    {
        FKashmirHitEvidence Evidence;
        Evidence.InstigatorId = TEXT("Attacker");
        Evidence.TargetId = TEXT("Defender");
        Evidence.ContactSource = EKashmirContactSourceType::Weapon;
        Evidence.SourceId = TEXT("Sword");
        Evidence.ImpactNormal = FVector::UpVector;
        Evidence.ContactVelocity = AttackDirection.GetSafeNormal() * 120.0f;
        Evidence.AttackDirection = AttackDirection;
        Evidence.RelativeSpeed = 120.0f;
        return Evidence;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDeflectResolverMatrixTest,
    "Kashmir.Combat.Deflect.ResolutionMatrix",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirDeflectResolverMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirDeflectResolver Resolver;
    const FKashmirHitEvidence Evidence = MakeDeflectEvidence(FVector(-2.0f, 0.0f, 0.0f));

    FKashmirDeflectInput Input;
    Input.ParryResult.bParried = true;
    Input.StrengthMultiplier = 1.5f;
    FKashmirDeflectResult Result;
    FString Reason;

    TestTrue(TEXT("Successful parry resolves"), Resolver.Resolve(Evidence, Input, Result, Reason));
    TestTrue(TEXT("Successful parry deflects"), Result.bDeflected);
    TestEqual(TEXT("Deflect outcome is explicit"), Result.Outcome, EKashmirDeflectOutcome::Deflect);
    TestTrue(TEXT("Reaction points toward attack origin"), Result.ReactionDirection.Equals(FVector::ForwardVector, KINDA_SMALL_NUMBER));
    TestEqual(TEXT("Multiplier scales physical intensity"), Result.PhysicalIntensity, 180.0f);

    Input.ParryResult.bParried = false;
    Result = {};
    TestTrue(TEXT("Failed parry still resolves"), Resolver.Resolve(Evidence, Input, Result, Reason));
    TestFalse(TEXT("Failed parry does not deflect"), Result.bDeflected);
    TestEqual(TEXT("Failed parry has zero intensity"), Result.PhysicalIntensity, 0.0f);

    Input.ParryResult.bParried = true;
    Input.StrengthMultiplier = 0.0f;
    Result = {};
    TestTrue(TEXT("Zero multiplier is valid"), Resolver.Resolve(Evidence, Input, Result, Reason));
    TestTrue(TEXT("Zero multiplier still represents deflect"), Result.bDeflected);
    TestEqual(TEXT("Zero multiplier produces zero intensity"), Result.PhysicalIntensity, 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDeflectResolverInvalidInputTest,
    "Kashmir.Combat.Deflect.RejectInvalidInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirDeflectResolverInvalidInputTest::RunTest(const FString& Parameters)
{
    const TArray<float> InvalidMultipliers =
    {
        -1.0f,
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity()
    };

    FKashmirDeflectResolver Resolver;
    for (const float Multiplier : InvalidMultipliers)
    {
        FKashmirDeflectInput Input;
        Input.ParryResult.bParried = true;
        Input.StrengthMultiplier = Multiplier;
        FKashmirDeflectResult Result;
        FString Reason;
        TestFalse(TEXT("Invalid multiplier is rejected"), Resolver.Resolve(MakeDeflectEvidence(), Input, Result, Reason));
        TestFalse(TEXT("Invalid multiplier provides reason"), Reason.IsEmpty());
    }

    FKashmirDeflectInput Input;
    Input.ParryResult.bParried = true;
    FKashmirDeflectResult Result;
    FString Reason;
    TestFalse(TEXT("Zero attack direction is rejected for a parry"), Resolver.Resolve(MakeDeflectEvidence(FVector::ZeroVector), Input, Result, Reason));
    TestTrue(TEXT("Zero direction rejection is explicit"), Reason.Contains(TEXT("attack direction")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDeflectDefensePipelineIntegrationTest,
    "Kashmir.Combat.Deflect.DefensePipelineIntegration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirDeflectDefensePipelineIntegrationTest::RunTest(const FString& Parameters)
{
    FKashmirDefensePipelineInput Input;
    Input.BlockState.bActive = true;
    Input.BlockState.ForwardDirection = FVector::ForwardVector;
    Input.BlockState.HalfAngleDegrees = 60.0f;
    Input.ParryState.bActive = true;
    Input.ParryState.ElapsedTimeSeconds = 0.2f;
    Input.ParryState.Window.StartTimeSeconds = 0.1f;
    Input.ParryState.Window.EndTimeSeconds = 0.3f;
    Input.DeflectStrengthMultiplier = 2.0f;
    Input.BaseGuardDamage = 90.0f;
    Input.AvailableStamina = 50.0f;

    FKashmirDefensePipelineResult Result;
    FString Reason;
    FKashmirDefensePipeline Pipeline;
    TestTrue(TEXT("Pipeline resolves parry and deflect"), Pipeline.Resolve(MakeDeflectEvidence(), Input, Result, Reason));
    TestTrue(TEXT("Pipeline preserves parry priority"), Result.bParried);
    TestTrue(TEXT("Pipeline exposes deflect"), Result.bDeflected);
    TestFalse(TEXT("Parry bypasses block"), Result.bBlocked);
    TestFalse(TEXT("Parry bypasses guard processing"), Result.Guard.bGuardProcessed);
    TestEqual(TEXT("Parry preserves stamina"), Result.Guard.StaminaAfter, 50.0f);
    TestEqual(TEXT("Pipeline applies deflect multiplier"), Result.Deflect.PhysicalIntensity, 240.0f);
    return true;
}

#endif
