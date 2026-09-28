#include "Combat/KashmirStaggerResolver.h"


bool FKashmirStaggerConfig::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(MinimumDeflectIntensity) ||
        MinimumDeflectIntensity < 0.0f)
    {
        OutReason = TEXT("minimum deflect intensity must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(DeflectDurationSeconds) ||
        DeflectDurationSeconds < 0.0f)
    {
        OutReason = TEXT("deflect stagger duration must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(GuardBreakDurationSeconds) ||
        GuardBreakDurationSeconds < 0.0f)
    {
        OutReason = TEXT("guard break stagger duration must be finite and non-negative");
        return false;
    }

    return true;
}


bool FKashmirStaggerInput::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!Config.IsValid(OutReason))
    {
        return false;
    }

    if (!FMath::IsFinite(Deflect.PhysicalIntensity) ||
        Deflect.PhysicalIntensity < 0.0f)
    {
        OutReason = TEXT("deflect physical intensity must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(Guard.FinalGuardDamage) ||
        Guard.FinalGuardDamage < 0.0f)
    {
        OutReason = TEXT("final guard damage must be finite and non-negative");
        return false;
    }

    return true;
}


bool FKashmirStaggerResolver::Resolve(
    const FKashmirStaggerInput& Input,
    FKashmirStaggerResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Input.IsValid(OutReason))
    {
        return false;
    }

    /*
     * Deflect has priority because it is produced by parry, which already
     * has priority over block and guard in the defense pipeline.
     */
    if (Input.Deflect.bDeflected &&
        Input.Deflect.PhysicalIntensity >= Input.Config.MinimumDeflectIntensity)
    {
        OutResult.bStaggered = true;
        OutResult.Cause = EKashmirStaggerCause::Deflect;
        OutResult.Target = EKashmirStaggerTarget::Attacker;
        OutResult.DurationSeconds = Input.Config.DeflectDurationSeconds;
        OutResult.PhysicalIntensity = Input.Deflect.PhysicalIntensity;
        return true;
    }

    if (Input.Guard.bGuardBroken)
    {
        OutResult.bStaggered = true;
        OutResult.Cause = EKashmirStaggerCause::GuardBreak;
        OutResult.Target = EKashmirStaggerTarget::Defender;
        OutResult.DurationSeconds = Input.Config.GuardBreakDurationSeconds;
        OutResult.PhysicalIntensity = Input.Guard.FinalGuardDamage;
    }

    return true;
}
