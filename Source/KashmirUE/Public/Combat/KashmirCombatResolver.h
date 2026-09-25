#pragma once

#include "CoreMinimal.h"
#include "Contracts/KashmirActionContracts.h"

struct KASHMIRUE_API FKashmirCombatResolveContext
{
    FName InstigatorId;
    TArray<FName> CandidateTargetIds;
};

class KASHMIRUE_API FKashmirCombatResolver
{
public:

    bool Resolve(
        const FKashmirCombatActionDefinition& Definition,
        const FKashmirCombatResolveContext& Context,
        FKashmirCombatResult& OutResult,
        FString& OutReason
    ) const;

private:

    bool ResolveDelivery(
        const FKashmirCombatActionDefinition& Definition,
        const FKashmirCombatResolveContext& Context,
        FKashmirCombatResult& OutResult,
        FString& OutReason
    ) const;

    bool ResolveTargets(
        const FKashmirCombatActionDefinition& Definition,
        const FKashmirCombatResolveContext& Context,
        FKashmirCombatResult& OutResult,
        FString& OutReason
    ) const;

    void ResolveEffects(
        const FKashmirCombatActionDefinition& Definition,
        FKashmirCombatResult& OutResult
    ) const;

    FGameplayTag ResolveEffectTag(
        EKashmirResolutionType Resolution
    ) const;
};