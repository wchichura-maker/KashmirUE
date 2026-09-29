#include "Combat/KashmirSwordRigAdapter.h"


namespace
{
    bool IsFiniteRigVector(const FVector& Value)
    {
        return
            FMath::IsFinite(Value.X) &&
            FMath::IsFinite(Value.Y) &&
            FMath::IsFinite(Value.Z);
    }

    bool IsFiniteRigRotator(const FRotator& Value)
    {
        return
            FMath::IsFinite(Value.Pitch) &&
            FMath::IsFinite(Value.Yaw) &&
            FMath::IsFinite(Value.Roll);
    }

    bool IsRigUnitInterval(float Value)
    {
        return FMath::IsFinite(Value) && Value >= 0.0f && Value <= 1.0f;
    }
}


bool FKashmirSwordRigAdapter::Resolve(
    const FKashmirSwordPoseResult& Pose,
    FKashmirSwordRigInputs& OutInputs,
    FString& OutReason) const
{
    OutInputs = {};
    OutReason.Reset();

    if (!Pose.bEnabled)
    {
        return true;
    }

    if (!IsFiniteRigVector(Pose.LeadHandOffset) ||
        !IsFiniteRigVector(Pose.SupportHandOffset) ||
        !IsFiniteRigRotator(Pose.AimRotation) ||
        !FMath::IsFinite(Pose.BodyLeanDegrees) ||
        !IsRigUnitInterval(Pose.PhaseAlpha) ||
        !IsRigUnitInterval(Pose.LeftFootLockAlpha) ||
        !IsRigUnitInterval(Pose.RightFootLockAlpha))
    {
        OutReason = TEXT("sword rig pose values must be finite and normalized");
        return false;
    }

    OutInputs.LeadHandOffset = Pose.LeadHandOffset;
    OutInputs.SupportHandOffset = Pose.SupportHandOffset;
    OutInputs.AimRotation = Pose.AimRotation;
    OutInputs.BodyLeanDegrees = Pose.BodyLeanDegrees;
    OutInputs.SwordPoseAlpha = 1.0f;
    OutInputs.LeftFootLockAlpha = Pose.LeftFootLockAlpha;
    OutInputs.RightFootLockAlpha = Pose.RightFootLockAlpha;
    return true;
}
