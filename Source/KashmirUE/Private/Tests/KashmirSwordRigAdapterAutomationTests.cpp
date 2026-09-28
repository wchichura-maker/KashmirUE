#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirSwordRigAdapter.h"
#include "Misc/AutomationTest.h"

#include <limits>


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigAdapterMatrixTest,
    "Kashmir.Animation.DirectionalSword.RigAdapter.Matrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigAdapterMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirSwordRigAdapter Adapter;
    FKashmirSwordPoseResult Pose;
    FKashmirSwordRigCurveValues Curves;
    FString Reason;

    TestTrue(TEXT("Disabled pose resolves"), Adapter.Resolve(Pose, Curves, Reason));
    TestEqual(TEXT("Disabled pose keeps rig neutral"), Curves.SwordPoseAlpha, 0.0f);

    Pose.bEnabled = true;
    Pose.PhaseAlpha = 0.5f;
    Pose.LeadHandOffset = FVector(4.0f, 3.0f, 2.0f);
    Pose.SupportHandOffset = FVector(1.0f, -2.0f, 3.0f);
    Pose.AimRotation = FRotator(-10.0f, 20.0f, 5.0f);
    Pose.BodyLeanDegrees = 7.0f;
    Pose.LeftFootLockAlpha = 0.75f;
    Pose.RightFootLockAlpha = 0.5f;

    TestTrue(TEXT("Enabled pose resolves"), Adapter.Resolve(Pose, Curves, Reason));
    TestEqual(TEXT("Lead hand offset is preserved"), Curves.LeadHandOffset, Pose.LeadHandOffset);
    TestEqual(TEXT("Support hand offset is preserved"), Curves.SupportHandOffset, Pose.SupportHandOffset);
    TestEqual(TEXT("Aim rotation is preserved"), Curves.AimRotation, Pose.AimRotation);
    TestEqual(TEXT("Body lean is preserved"), Curves.BodyLeanDegrees, 7.0f);
    TestEqual(TEXT("Enabled pose activates rig once"), Curves.SwordPoseAlpha, 1.0f);
    TestEqual(TEXT("Left foot weight is preserved"), Curves.LeftFootLockAlpha, 0.75f);
    TestEqual(TEXT("Right foot weight is preserved"), Curves.RightFootLockAlpha, 0.5f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigAdapterRejectInvalidTest,
    "Kashmir.Animation.DirectionalSword.RigAdapter.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigAdapterRejectInvalidTest::RunTest(const FString& Parameters)
{
    FKashmirSwordRigAdapter Adapter;
    FKashmirSwordPoseResult Pose;
    Pose.bEnabled = true;
    Pose.PhaseAlpha = 1.0f;
    Pose.LeftFootLockAlpha = 1.0f;
    Pose.RightFootLockAlpha = 1.0f;
    FKashmirSwordRigCurveValues Curves;
    FString Reason;

    Pose.LeadHandOffset.X = std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("Non-finite hand target is rejected"), Adapter.Resolve(Pose, Curves, Reason));
    TestEqual(TEXT("Rejected pose resets rig"), Curves.SwordPoseAlpha, 0.0f);

    Pose.LeadHandOffset = FVector::ZeroVector;
    Pose.LeftFootLockAlpha = 1.1f;
    TestFalse(TEXT("Out-of-range foot lock is rejected"), Adapter.Resolve(Pose, Curves, Reason));
    return true;
}

#endif
