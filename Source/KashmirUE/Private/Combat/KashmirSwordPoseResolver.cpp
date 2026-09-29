#include "Combat/KashmirSwordPoseResolver.h"


namespace
{
    bool IsFinitePoseVector(const FVector& Value)
    {
        return
            FMath::IsFinite(Value.X) &&
            FMath::IsFinite(Value.Y) &&
            FMath::IsFinite(Value.Z);
    }

    bool IsPoseUnitInterval(float Value)
    {
        return FMath::IsFinite(Value) && Value >= 0.0f && Value <= 1.0f;
    }

    FVector ClampMagnitude(const FVector& Value, float Maximum)
    {
        return Value.GetClampedToMaxSize(Maximum);
    }

    float ResolvePhaseAlpha(
        const FKashmirActionDefinition& Definition,
        const FKashmirActionRuntimeState& RuntimeState)
    {
        switch (RuntimeState.Phase)
        {
        case EKashmirActionPhase::Startup:
            return Definition.StartupDuration > UE_SMALL_NUMBER
                ? FMath::Clamp(RuntimeState.Elapsed / Definition.StartupDuration, 0.0f, 1.0f)
                : 1.0f;

        case EKashmirActionPhase::Active:
            return 1.0f;

        case EKashmirActionPhase::Recovery:
        {
            const float RecoveryElapsed = FMath::Max(
                0.0f,
                RuntimeState.Elapsed - Definition.StartupDuration - Definition.ActiveDuration);
            return Definition.RecoveryDuration > UE_SMALL_NUMBER
                ? 1.0f - FMath::Clamp(RecoveryElapsed / Definition.RecoveryDuration, 0.0f, 1.0f)
                : 0.0f;
        }

        default:
            return 0.0f;
        }
    }

    float ResolveFootLock(
        const FKashmirSwordPoseConfig& Config,
        EKashmirActionPhase Phase)
    {
        switch (Phase)
        {
        case EKashmirActionPhase::Startup:
            return Config.StartupFootLockAlpha;
        case EKashmirActionPhase::Active:
            return Config.ActiveFootLockAlpha;
        case EKashmirActionPhase::Recovery:
            return Config.RecoveryFootLockAlpha;
        default:
            return 0.0f;
        }
    }
}


bool FKashmirSwordPoseConfig::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!IsFinitePoseVector(LeadHandOffsetAtFullIntensity) ||
        !IsFinitePoseVector(SupportHandOffsetAtFullIntensity))
    {
        OutReason = TEXT("sword pose hand offsets must be finite");
        return false;
    }

    if (!FMath::IsFinite(MaximumHandOffset) || MaximumHandOffset < 0.0f ||
        !FMath::IsFinite(AimYawAtFullIntensity) ||
        !FMath::IsFinite(AimPitchAtFullIntensity) ||
        !FMath::IsFinite(MaximumAimYaw) || MaximumAimYaw < 0.0f ||
        !FMath::IsFinite(MaximumAimPitch) || MaximumAimPitch < 0.0f ||
        !FMath::IsFinite(MaximumWeaponRoll) || MaximumWeaponRoll < 0.0f ||
        !FMath::IsFinite(MaximumBodyLean) || MaximumBodyLean < 0.0f)
    {
        OutReason = TEXT("sword pose limits must be finite and non-negative");
        return false;
    }

    if (!IsPoseUnitInterval(StartupFootLockAlpha) ||
        !IsPoseUnitInterval(ActiveFootLockAlpha) ||
        !IsPoseUnitInterval(RecoveryFootLockAlpha))
    {
        OutReason = TEXT("sword pose foot lock weights must be between zero and one");
        return false;
    }

    return true;
}


bool FKashmirSwordPoseResolver::Resolve(
    const FKashmirDirectionalSwordResult& Gesture,
    const FKashmirActionDefinition& Definition,
    const FKashmirSwordPoseConfig& Config,
    const FKashmirActionRuntimeState& RuntimeState,
    FKashmirSwordPoseResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Gesture.bGestureResolved || !Gesture.ActionRequest.IsValid(OutReason))
    {
        OutReason = OutReason.IsEmpty()
            ? TEXT("sword pose requires a resolved gesture")
            : FString::Printf(TEXT("invalid sword pose gesture: %s"), *OutReason);
        return false;
    }

    if (!Definition.IsValid(OutReason))
    {
        OutReason = FString::Printf(TEXT("invalid sword pose action definition: %s"), *OutReason);
        return false;
    }

    if (!Config.IsValid(OutReason))
    {
        OutReason = FString::Printf(TEXT("invalid sword pose config: %s"), *OutReason);
        return false;
    }

    if (!FMath::IsFinite(RuntimeState.Elapsed) || RuntimeState.Elapsed < 0.0f)
    {
        OutReason = TEXT("sword pose runtime elapsed time must be finite and non-negative");
        return false;
    }

    if (!RuntimeState.bActive || RuntimeState.ActionId != Definition.ActionId)
    {
        return true;
    }

    const FVector2D Direction = Gesture.ActionRequest.Direction.GetSafeNormal();
    if (!FMath::IsFinite(Direction.X) || !FMath::IsFinite(Direction.Y) || Direction.IsNearlyZero())
    {
        OutReason = TEXT("sword pose gesture direction must be finite and non-zero");
        return false;
    }

    const float Intensity = FMath::Clamp(Gesture.ActionRequest.Intensity, 0.0f, 1.0f);
    OutResult.PhaseAlpha = ResolvePhaseAlpha(Definition, RuntimeState);
    const float PoseWeight = Intensity * OutResult.PhaseAlpha;

    OutResult.LeadHandOffset = ClampMagnitude(
        Config.LeadHandOffsetAtFullIntensity * PoseWeight,
        Config.MaximumHandOffset);
    OutResult.SupportHandOffset = ClampMagnitude(
        Config.SupportHandOffsetAtFullIntensity * PoseWeight,
        Config.MaximumHandOffset);

    OutResult.AimRotation.Yaw = FMath::Clamp(
        Direction.X * Config.AimYawAtFullIntensity * PoseWeight,
        -Config.MaximumAimYaw,
        Config.MaximumAimYaw);
    OutResult.AimRotation.Pitch = FMath::Clamp(
        -Direction.Y * Config.AimPitchAtFullIntensity * PoseWeight,
        -Config.MaximumAimPitch,
        Config.MaximumAimPitch);
    const float RollSign = Direction.X < 0.0f ? -1.0f : 1.0f;
    OutResult.AimRotation.Roll =
        RollSign * Gesture.ActionRequest.Curvature * Config.MaximumWeaponRoll * OutResult.PhaseAlpha;
    OutResult.BodyLeanDegrees = FMath::Clamp(
        Direction.X * Config.MaximumBodyLean * PoseWeight,
        -Config.MaximumBodyLean,
        Config.MaximumBodyLean);

    const float FootLock = ResolveFootLock(Config, RuntimeState.Phase);
    OutResult.LeftFootLockAlpha = FootLock;
    OutResult.RightFootLockAlpha = FootLock;
    OutResult.bEnabled = OutResult.PhaseAlpha > 0.0f;
    return true;
}
