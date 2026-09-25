#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirHitEvidence.h"
#include "Combat/KashmirWeaponTraceComponent.h"
#include "Contracts/KashmirActionContracts.h"

class UKashmirHitRegionMap;

struct KASHMIRUE_API FKashmirMeleeHitProcessInput
{
    FName InstigatorId;
    FName TargetId;
    FName SourceId;

    const UKashmirHitRegionMap* HitRegionMap = nullptr;

    FGameplayTagContainer EvidenceTags;
        /** Base damage supplied by the current attack/weapon definition. */
    float BaseDamage = 0.0f;

    /** Offensive multiplier resolved by gameplay systems. */
    float AttackPowerMultiplier = 1.0f;

    /**
     * Regional scaling supplied by combat data.
     * The semantic HitRegion itself still comes from physical evidence.
     */
    float RegionMultiplier = 1.0f;
};

struct KASHMIRUE_API FKashmirMeleeHitProcessResult
{
    FKashmirHitEvidence Evidence;
    FKashmirCombatResult CombatResult;
};

class KASHMIRUE_API FKashmirMeleeHitProcessor
{
public:

    bool Process(
        const FKashmirWeaponTraceHit& TraceHit,
        const FKashmirCombatActionDefinition& Definition,
        const FKashmirMeleeHitProcessInput& Input,
        FKashmirMeleeHitProcessResult& OutResult,
        FString& OutReason
    ) const;
};