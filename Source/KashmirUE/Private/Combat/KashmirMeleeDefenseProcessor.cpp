#include "Combat/KashmirMeleeDefenseProcessor.h"


bool FKashmirMeleeDefenseProcessor::Resolve(
    const FKashmirMeleeHitProcessResult& HitResult,
    const FKashmirMeleeDefenseInput& Input,
    FKashmirMeleeDefenseResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    OutResult.HitResult =
        HitResult;

    FKashmirDefensePipeline Pipeline;

    if (!Pipeline.Resolve(
            HitResult.Evidence,
            Input.DefenseInput,
            OutResult.DefenseResult,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    OutResult.bDamageSuppressedByBlock =
        OutResult.DefenseResult.bBlocked;

    OutResult.bDamageAllowed =
        !OutResult.bDamageSuppressedByBlock;

    /*
    * Preserve the already resolved damage magnitude,
    * but mark health-damage effects as suppressed
    * whenever the contact was successfully blocked.
    *
    * Guard Break does not retroactively allow this
    * same hit through the guard.
    */
    for (FKashmirEffectResult& Effect :
        OutResult.HitResult.CombatResult.Effects)
    {
        if (Effect.Resolution ==
            EKashmirResolutionType::Damage)
        {
            Effect.bSuppressed =
                OutResult.bDamageSuppressedByBlock;
        }
    }

    return true;
}