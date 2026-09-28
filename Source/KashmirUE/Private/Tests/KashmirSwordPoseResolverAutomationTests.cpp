#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirSwordPoseResolver.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirDirectionalSwordResult MakePoseGesture()
    {
        FKashmirDirectionalSwordResult Gesture;
        Gesture.bGestureResolved = true;
        Gesture.ActionRequest.ActionId = TEXT("Sword.Direct.Right");
        Gesture.ActionRequest.Direction = FVector2D(1.0f, 0.0f);
        Gesture.ActionRequest.Intensity = 1.0f;
        Gesture.ActionRequest.Curvature = 0.5f;
        return Gesture;
    }

    FKashmirActionDefinition MakePoseDefinition()
    {
        FKashmirActionDefinition Definition;
        Definition.ActionId = TEXT("Sword.Direct.Right");
        Definition.StartupDuration = 0.20f;
        Definition.ActiveDuration = 0.20f;
        Definition.RecoveryDuration = 0.40f;
        return Definition;
    }

    FKashmirSwordPoseConfig MakePoseConfig()
    {
        FKashmirSwordPoseConfig Config;
        Config.LeadHandOffsetAtFullIntensity = FVector(100.0f, 0.0f, 0.0f);
        Config.SupportHandOffsetAtFullIntensity = FVector(8.0f, 0.0f, 0.0f);
        Config.MaximumHandOffset = 10.0f;
        Config.AimYawAtFullIntensity = 100.0f;
        Config.MaximumAimYaw = 30.0f;
        Config.MaximumWeaponRoll = 20.0f;
        Config.MaximumBodyLean = 10.0f;
        Config.StartupFootLockAlpha = 0.90f;
        Config.ActiveFootLockAlpha = 0.50f;
        Config.RecoveryFootLockAlpha = 0.80f;
        return Config;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordPosePhaseMatrixTest,
    "Kashmir.Combat.DirectionalSword.Pose.PhaseMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordPosePhaseMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirSwordPoseResolver Resolver;
    const FKashmirDirectionalSwordResult Gesture = MakePoseGesture();
    const FKashmirActionDefinition Definition = MakePoseDefinition();
    const FKashmirSwordPoseConfig Config = MakePoseConfig();
    FKashmirActionRuntimeState RuntimeState;
    RuntimeState.ActionId = Definition.ActionId;
    RuntimeState.bActive = true;
    FKashmirSwordPoseResult Result;
    FString Reason;

    RuntimeState.Phase = EKashmirActionPhase::Startup;
    RuntimeState.Elapsed = 0.10f;
    TestTrue(TEXT("Startup pose resolves"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));
    TestTrue(TEXT("Startup pose is enabled"), Result.bEnabled);
    TestEqual(TEXT("Startup ramps pose weight"), Result.PhaseAlpha, 0.5f);
    TestEqual(TEXT("Hand reach obeys anatomical limit"), Result.LeadHandOffset.Size(), 10.0);
    TestEqual(TEXT("Aim obeys yaw limit"), Result.AimRotation.Yaw, 30.0);
    TestEqual(TEXT("Curvature drives weapon roll"), Result.AimRotation.Roll, 5.0);
    TestEqual(TEXT("Startup foot lock is authored"), Result.LeftFootLockAlpha, 0.90f);

    RuntimeState.Phase = EKashmirActionPhase::Active;
    RuntimeState.Elapsed = 0.30f;
    TestTrue(TEXT("Active pose resolves"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));
    TestEqual(TEXT("Active pose reaches full weight"), Result.PhaseAlpha, 1.0f);
    TestEqual(TEXT("Active foot lock is authored"), Result.RightFootLockAlpha, 0.50f);

    RuntimeState.Phase = EKashmirActionPhase::Recovery;
    RuntimeState.Elapsed = 0.60f;
    TestTrue(TEXT("Recovery pose resolves"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));
    TestEqual(TEXT("Recovery ramps pose weight down"), Result.PhaseAlpha, 0.5f);
    TestEqual(TEXT("Recovery foot lock is authored"), Result.LeftFootLockAlpha, 0.80f);

    RuntimeState.bActive = false;
    RuntimeState.Phase = EKashmirActionPhase::Complete;
    TestTrue(TEXT("Inactive pose resolves"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));
    TestFalse(TEXT("Inactive runtime disables rig"), Result.bEnabled);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordPoseDirectionTest,
    "Kashmir.Combat.DirectionalSword.Pose.DirectionAndIntensity",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordPoseDirectionTest::RunTest(const FString& Parameters)
{
    FKashmirSwordPoseResolver Resolver;
    FKashmirDirectionalSwordResult Gesture = MakePoseGesture();
    Gesture.ActionRequest.Direction = FVector2D(-1.0f, 1.0f).GetSafeNormal();
    Gesture.ActionRequest.Intensity = 0.5f;
    const FKashmirActionDefinition Definition = MakePoseDefinition();
    const FKashmirSwordPoseConfig Config = MakePoseConfig();
    FKashmirActionRuntimeState RuntimeState;
    RuntimeState.ActionId = Definition.ActionId;
    RuntimeState.Phase = EKashmirActionPhase::Active;
    RuntimeState.Elapsed = 0.30f;
    RuntimeState.bActive = true;
    FKashmirSwordPoseResult Result;
    FString Reason;

    TestTrue(TEXT("Directional pose resolves"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));
    TestTrue(TEXT("Left gesture produces negative yaw"), Result.AimRotation.Yaw < 0.0f);
    TestTrue(TEXT("Up gesture produces negative local pitch"), Result.AimRotation.Pitch < 0.0f);
    TestTrue(TEXT("Left curved gesture produces negative roll"), Result.AimRotation.Roll < 0.0f);
    TestTrue(TEXT("Left gesture leans left"), Result.BodyLeanDegrees < 0.0f);
    TestEqual(TEXT("Intensity scales support hand"), Result.SupportHandOffset.Size(), 4.0);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordPoseRejectInvalidTest,
    "Kashmir.Combat.DirectionalSword.Pose.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordPoseRejectInvalidTest::RunTest(const FString& Parameters)
{
    FKashmirSwordPoseResolver Resolver;
    FKashmirDirectionalSwordResult Gesture = MakePoseGesture();
    const FKashmirActionDefinition Definition = MakePoseDefinition();
    FKashmirSwordPoseConfig Config = MakePoseConfig();
    FKashmirActionRuntimeState RuntimeState;
    RuntimeState.ActionId = Definition.ActionId;
    RuntimeState.Phase = EKashmirActionPhase::Active;
    RuntimeState.Elapsed = 0.30f;
    RuntimeState.bActive = true;
    FKashmirSwordPoseResult Result;
    FString Reason;

    Gesture.bGestureResolved = false;
    TestFalse(TEXT("Unresolved gesture is rejected"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));

    Gesture = MakePoseGesture();
    Gesture.ActionRequest.Direction = FVector2D::ZeroVector;
    TestFalse(TEXT("Zero direction is rejected"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));

    Gesture = MakePoseGesture();
    Config.MaximumHandOffset = -1.0f;
    TestFalse(TEXT("Negative anatomical limit is rejected"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));

    Config = MakePoseConfig();
    RuntimeState.Elapsed = std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("Invalid runtime time is rejected"), Resolver.Resolve(Gesture, Definition, Config, RuntimeState, Result, Reason));
    return true;
}

#endif
