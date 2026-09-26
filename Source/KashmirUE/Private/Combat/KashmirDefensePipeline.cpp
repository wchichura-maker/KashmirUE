#include "Combat/KashmirDefensePipeline.h"


bool FKashmirDefensePipeline::Resolve(
    const FKashmirHitEvidence& Evidence,
    const FKashmirDefensePipelineInput& Input,
    FKashmirDefensePipelineResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

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