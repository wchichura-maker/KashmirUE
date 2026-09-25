#include "Combat/KashmirMeleeHitProcessor.h"

#include "Combat/KashmirCombatResolver.h"
#include "Combat/KashmirHitEvidenceBuilder.h"
#include "Combat/KashmirHitRegionMap.h"
#include "Combat/KashmirDamageResolver.h"

bool FKashmirMeleeHitProcessor::Process(
    const FKashmirWeaponTraceHit& TraceHit,
    const FKashmirCombatActionDefinition& Definition,
    const FKashmirMeleeHitProcessInput& Input,
    FKashmirMeleeHitProcessResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (Input.InstigatorId.IsNone())
    {
        OutReason =
            TEXT("melee hit requires instigator id");

        return false;
    }

    if (Input.TargetId.IsNone())
    {
        OutReason =
            TEXT("melee hit requires target id");

        return false;
    }

    if (Input.SourceId.IsNone())
    {
        OutReason =
            TEXT("melee hit requires source id");

        return false;
    }

    if (Definition.Delivery !=
        EKashmirDeliveryType::Contact)
    {
        OutReason =
            TEXT("melee hit requires contact delivery");

        return false;
    }

    FKashmirHitEvidenceBuildInput EvidenceInput;

    EvidenceInput.InstigatorId =
        Input.InstigatorId;

    EvidenceInput.TargetId =
        Input.TargetId;

    EvidenceInput.ContactSource =
        EKashmirContactSourceType::Weapon;

    EvidenceInput.SourceId =
        Input.SourceId;

    EvidenceInput.ContactVelocity =
        TraceHit.ContactVelocity;

    EvidenceInput.AttackDirection =
        TraceHit.AttackDirection;

    EvidenceInput.RelativeSpeed =
        TraceHit.Speed;

    EvidenceInput.EvidenceTags =
        Input.EvidenceTags;

    if (!FKashmirHitEvidenceBuilder::BuildFromHitResult(
            TraceHit.Hit,
            EvidenceInput,
            Input.HitRegionMap,
            OutResult.Evidence,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    FKashmirCombatResolveContext ResolveContext;

    ResolveContext.InstigatorId =
        Input.InstigatorId;

    ResolveContext.CandidateTargetIds.Add(
        Input.TargetId
    );

    FKashmirCombatResolver Resolver;

    if (!Resolver.Resolve(
            Definition,
            ResolveContext,
            OutResult.CombatResult,
            OutReason))
    {
        OutResult = {};
        return false;
    }
    for (FKashmirEffectResult& Effect :
        OutResult.CombatResult.Effects)
    {
        Effect.HitRegion =
            OutResult.Evidence.HitRegion;

        Effect.ImpactDirection =
            OutResult.Evidence.AttackDirection;

        Effect.ImpactStrength =
            OutResult.Evidence.RelativeSpeed;

        if (Effect.Resolution ==
            EKashmirResolutionType::Damage)
        {
            FKashmirDamageInput DamageInput;

            DamageInput.BaseDamage =
                Input.BaseDamage;

            DamageInput.AttackPowerMultiplier =
                Input.AttackPowerMultiplier;

            DamageInput.RegionMultiplier =
                Input.RegionMultiplier;

            DamageInput.HitRegion =
                OutResult.Evidence.HitRegion;

            FKashmirDamageResult DamageResult;

            FKashmirDamageResolver DamageResolver;

            if (!DamageResolver.Resolve(
                    DamageInput,
                    DamageResult,
                    OutReason))
            {
                OutResult = {};
                return false;
            }

            Effect.Magnitude =
                DamageResult.FinalDamage;
        }
    }

    return true;
}