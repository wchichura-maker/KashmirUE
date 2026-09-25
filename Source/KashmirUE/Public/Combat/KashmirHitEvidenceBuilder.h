#pragma once

#include "CoreMinimal.h"
#include "Combat/KashmirHitEvidence.h"

struct FHitResult;
class UKashmirHitRegionMap;

struct KASHMIRUE_API FKashmirHitEvidenceBuildInput
{
    FName InstigatorId;
    FName TargetId;

    EKashmirContactSourceType ContactSource =
        EKashmirContactSourceType::Unknown;

    FName SourceId;

    FVector ContactVelocity =
        FVector::ZeroVector;

    FVector AttackDirection =
        FVector::ZeroVector;

    float RelativeSpeed = 0.0f;

    FGameplayTagContainer EvidenceTags;
};

class KASHMIRUE_API FKashmirHitEvidenceBuilder
{
public:

    static bool BuildFromHitResult(
        const FHitResult& HitResult,
        const FKashmirHitEvidenceBuildInput& Input,
        const UKashmirHitRegionMap* HitRegionMap,
        FKashmirHitEvidence& OutEvidence,
        FString& OutReason
    );
};