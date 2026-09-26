#include "Combat/KashmirGuardResolver.h"


bool FKashmirGuardInput::IsValid(
    FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(BaseGuardDamage))
    {
        OutReason =
            TEXT(
                "base guard damage must be finite"
            );

        return false;
    }

    if (BaseGuardDamage < 0.0f)
    {
        OutReason =
            TEXT(
                "base guard damage must be non-negative"
            );

        return false;
    }

    if (!FMath::IsFinite(
            GuardDamageMultiplier))
    {
        OutReason =
            TEXT(
                "guard damage multiplier must be finite"
            );

        return false;
    }

    if (GuardDamageMultiplier < 0.0f)
    {
        OutReason =
            TEXT(
                "guard damage multiplier must be non-negative"
            );

        return false;
    }

    if (!FMath::IsFinite(
            AvailableStamina))
    {
        OutReason =
            TEXT(
                "available stamina must be finite"
            );

        return false;
    }

    if (AvailableStamina < 0.0f)
    {
        OutReason =
            TEXT(
                "available stamina must be non-negative"
            );

        return false;
    }

    return true;
}


bool FKashmirGuardResolver::Resolve(
    const FKashmirGuardInput& Input,
    FKashmirGuardResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Input.IsValid(
            OutReason))
    {
        return false;
    }

    OutResult.StaminaBefore =
        Input.AvailableStamina;

    /*
     * No successful geometric block means
     * guard stamina is not processed.
     */
    if (!Input.bBlocked)
    {
        OutResult.StaminaAfter =
            Input.AvailableStamina;

        OutResult.bGuardProcessed =
            false;

        OutResult.bGuardBroken =
            false;

        return true;
    }

    OutResult.RawGuardDamage =
        Input.BaseGuardDamage;

    OutResult.FinalGuardDamage =
        Input.BaseGuardDamage *
        Input.GuardDamageMultiplier;

    if (!FMath::IsFinite(
            OutResult.FinalGuardDamage))
    {
        OutReason =
            TEXT(
                "resolved guard damage must be finite"
            );

        OutResult = {};

        return false;
    }

    OutResult.AppliedStaminaDamage =
        FMath::Min(
            Input.AvailableStamina,
            OutResult.FinalGuardDamage
        );

    OutResult.StaminaAfter =
        FMath::Max(
            0.0f,
            Input.AvailableStamina -
            OutResult.FinalGuardDamage
        );

    /*
     * Exact stamina is enough to absorb
     * the block without guard break.
     *
     * Guard break occurs only when the
     * required guard damage is greater
     * than available stamina.
     */
    OutResult.bGuardBroken =
        OutResult.FinalGuardDamage >
        Input.AvailableStamina;

    OutResult.bGuardProcessed =
        true;

    return true;
}