#include "Combat/KashmirDamageResolver.h"

bool FKashmirDamageInput::IsValid(
    FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(BaseDamage) ||
        BaseDamage < 0.0f)
    {
        OutReason =
            TEXT("base damage must be non-negative");

        return false;
    }

    if (!FMath::IsFinite(AttackPowerMultiplier) ||
        AttackPowerMultiplier < 0.0f)
    {
        OutReason =
            TEXT("attack power multiplier must be non-negative");

        return false;
    }

    if (!FMath::IsFinite(RegionMultiplier) ||
        RegionMultiplier < 0.0f)
    {
        OutReason =
            TEXT("region multiplier must be non-negative");

        return false;
    }

    if (!HitRegion.IsValid())
    {
        OutReason =
            TEXT("damage resolution requires hit region");

        return false;
    }

    return true;
}

bool FKashmirDamageResolver::Resolve(
    const FKashmirDamageInput& Input,
    FKashmirDamageResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Input.IsValid(
            OutReason))
    {
        return false;
    }

    const float RawDamage =
        Input.BaseDamage *
        Input.AttackPowerMultiplier;

    const float FinalDamage =
        RawDamage *
        Input.RegionMultiplier;

    if (!FMath::IsFinite(RawDamage) ||
        !FMath::IsFinite(FinalDamage))
    {
        OutReason =
            TEXT("resolved damage must be finite");

        return false;
    }

    OutResult.RawDamage =
        RawDamage;

    OutResult.FinalDamage =
        FinalDamage;

    OutResult.HitRegion =
        Input.HitRegion;

    return true;
}