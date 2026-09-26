#include "Combat/KashmirDefensePipeline.h"


bool FKashmirDefensePipeline::Resolve(
    const FKashmirHitEvidence& Evidence,
    const FKashmirDefensePipelineInput& Input,
    FKashmirDefensePipelineResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    FKashmirParryResolver ParryResolver;

    if (!ParryResolver.Resolve(
            Evidence,
            Input.BlockState,
            Input.ParryState,
            OutResult.Parry,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    OutResult.bParried = OutResult.Parry.bParried;

    FKashmirDeflectInput DeflectInput;
    DeflectInput.ParryResult = OutResult.Parry;
    DeflectInput.StrengthMultiplier = Input.DeflectStrengthMultiplier;

    FKashmirDeflectResolver DeflectResolver;
    if (!DeflectResolver.Resolve(
            Evidence,
            DeflectInput,
            OutResult.Deflect,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    OutResult.bDeflected = OutResult.Deflect.bDeflected;

    /*
     * Parry has first priority. A successful parry ends defensive
     * resolution before block and guard, so baseline guard stamina
     * remains untouched.
     */
    if (OutResult.bParried)
    {
        OutResult.Guard.StaminaBefore = Input.AvailableStamina;
        OutResult.Guard.StaminaAfter = Input.AvailableStamina;
        return true;
    }

    FKashmirDefenseResolver DefenseResolver;

    if (!DefenseResolver.ResolveBlock(
            Evidence,
            Input.BlockState,
            OutResult.Defense,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    OutResult.bBlocked =
        OutResult.Defense.bBlocked;

    FKashmirGuardInput GuardInput;

    GuardInput.bBlocked =
        OutResult.Defense.bBlocked;

    GuardInput.BaseGuardDamage =
        Input.BaseGuardDamage;

    GuardInput.GuardDamageMultiplier =
        Input.GuardDamageMultiplier;

    GuardInput.AvailableStamina =
        Input.AvailableStamina;

    FKashmirGuardResolver GuardResolver;

    if (!GuardResolver.Resolve(
            GuardInput,
            OutResult.Guard,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    OutResult.bGuardBroken =
        OutResult.Guard.bGuardBroken;

    return true;
}
