#include "Combat/KashmirDeflectResolver.h"


bool FKashmirDeflectInput::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(StrengthMultiplier) ||
        StrengthMultiplier < 0.0f)
    {
        OutReason = TEXT("deflect strength multiplier must be finite and non-negative");
        return false;
    }

    return true;
}


bool FKashmirDeflectResolver::Resolve(
    const FKashmirHitEvidence& Evidence,
    const FKashmirDeflectInput& Input,
    FKashmirDeflectResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    FString ValidationReason;
    if (!Evidence.IsValid(ValidationReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid hit evidence: %s"),
            *ValidationReason);
        return false;
    }

    if (!Input.IsValid(ValidationReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid deflect input: %s"),
            *ValidationReason);
        return false;
    }

    if (!Input.ParryResult.bParried)
    {
        return true;
    }

    if (Evidence.AttackDirection.IsNearlyZero())
    {
        OutReason = TEXT("attack direction must not be zero for deflect");
        return false;
    }

    OutResult.bDeflected = true;
    OutResult.Outcome = EKashmirDeflectOutcome::Deflect;
    OutResult.ReactionDirection = -Evidence.AttackDirection.GetSafeNormal();
    OutResult.PhysicalIntensity = Evidence.RelativeSpeed * Input.StrengthMultiplier;
    return true;
}
