#include "Combat/KashmirSwordRigAdapter.h"


namespace KashmirSwordRigCurves
{
    const FName LeadHandOffsetX(TEXT("Kashmir.Sword.LeadHandOffsetX"));
    const FName LeadHandOffsetY(TEXT("Kashmir.Sword.LeadHandOffsetY"));
    const FName LeadHandOffsetZ(TEXT("Kashmir.Sword.LeadHandOffsetZ"));
    const FName SupportHandOffsetX(TEXT("Kashmir.Sword.SupportHandOffsetX"));
    const FName SupportHandOffsetY(TEXT("Kashmir.Sword.SupportHandOffsetY"));
    const FName SupportHandOffsetZ(TEXT("Kashmir.Sword.SupportHandOffsetZ"));
    const FName AimPitch(TEXT("Kashmir.Sword.AimPitch"));
    const FName AimYaw(TEXT("Kashmir.Sword.AimYaw"));
    const FName AimRoll(TEXT("Kashmir.Sword.AimRoll"));
    const FName BodyLean(TEXT("Kashmir.Sword.BodyLean"));
    const FName SwordPoseAlpha(TEXT("Kashmir.Sword.PoseAlpha"));
    const FName LeftFootLockAlpha(TEXT("Kashmir.Sword.LeftFootLockAlpha"));
    const FName RightFootLockAlpha(TEXT("Kashmir.Sword.RightFootLockAlpha"));
}


namespace
{
    bool IsFiniteVector(const FVector& Value)
    {
        return
            FMath::IsFinite(Value.X) &&
            FMath::IsFinite(Value.Y) &&
            FMath::IsFinite(Value.Z);
    }

    bool IsFiniteRotator(const FRotator& Value)
    {
        return
            FMath::IsFinite(Value.Pitch) &&
            FMath::IsFinite(Value.Yaw) &&
            FMath::IsFinite(Value.Roll);
    }

    bool IsUnitInterval(float Value)
    {
        return FMath::IsFinite(Value) && Value >= 0.0f && Value <= 1.0f;
    }
}


bool FKashmirSwordRigAdapter::Resolve(
    const FKashmirSwordPoseResult& Pose,
    FKashmirSwordRigCurveValues& OutCurves,
    FString& OutReason) const
{
    OutCurves = {};
    OutReason.Reset();

    if (!Pose.bEnabled)
    {
        return true;
    }

    if (!IsFiniteVector(Pose.LeadHandOffset) ||
        !IsFiniteVector(Pose.SupportHandOffset) ||
        !IsFiniteRotator(Pose.AimRotation) ||
        !FMath::IsFinite(Pose.BodyLeanDegrees) ||
        !IsUnitInterval(Pose.PhaseAlpha) ||
        !IsUnitInterval(Pose.LeftFootLockAlpha) ||
        !IsUnitInterval(Pose.RightFootLockAlpha))
    {
        OutReason = TEXT("sword rig pose values must be finite and normalized");
        return false;
    }

    OutCurves.LeadHandOffset = Pose.LeadHandOffset;
    OutCurves.SupportHandOffset = Pose.SupportHandOffset;
    OutCurves.AimRotation = Pose.AimRotation;
    OutCurves.BodyLeanDegrees = Pose.BodyLeanDegrees;
    OutCurves.SwordPoseAlpha = 1.0f;
    OutCurves.LeftFootLockAlpha = Pose.LeftFootLockAlpha;
    OutCurves.RightFootLockAlpha = Pose.RightFootLockAlpha;
    return true;
}
