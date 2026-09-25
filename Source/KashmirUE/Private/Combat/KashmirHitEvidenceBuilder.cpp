#include "Combat/KashmirHitEvidenceBuilder.h"

#include "Combat/KashmirHitRegionMap.h"
#include "Engine/HitResult.h"

bool FKashmirHitEvidenceBuilder::BuildFromHitResult(
    const FHitResult& HitResult,
    const FKashmirHitEvidenceBuildInput& Input,
    const UKashmirHitRegionMap* HitRegionMap,
    FKashmirHitEvidence& OutEvidence,
    FString& OutReason)
{
    OutEvidence = {};
    OutReason.Reset();

    if (!HitResult.bBlockingHit)
    {
        OutReason =
            TEXT("hit evidence requires a blocking hit");

        return false;
    }

    OutEvidence.InstigatorId =
        Input.InstigatorId;

    OutEvidence.TargetId =
        Input.TargetId;

    OutEvidence.ContactSource =
        Input.ContactSource;

    OutEvidence.SourceId =
        Input.SourceId;

    OutEvidence.ImpactPoint =
        HitResult.ImpactPoint;

    OutEvidence.ImpactNormal =
        HitResult.ImpactNormal;

    OutEvidence.ContactVelocity =
        Input.ContactVelocity;

    OutEvidence.AttackDirection =
        Input.AttackDirection.GetSafeNormal();

    OutEvidence.RelativeSpeed =
        Input.RelativeSpeed;

    OutEvidence.HitBone =
        HitResult.BoneName;

    OutEvidence.EvidenceTags =
        Input.EvidenceTags;

    // Skeletal contacts must resolve their skeleton-specific
    // bone name into a semantic Kashmir combat region.
    if (!HitResult.BoneName.IsNone())
    {
        if (HitRegionMap == nullptr)
        {
            OutReason =
                TEXT("bone hit requires hit region map");

            OutEvidence = {};
            return false;
        }

        if (!HitRegionMap->ResolveRegion(
                HitResult.BoneName,
                OutEvidence.HitRegion))
        {
            OutReason =
                TEXT("hit bone is not mapped to a combat region");

            OutEvidence = {};
            return false;
        }
    }

    if (!OutEvidence.IsValid(OutReason))
    {
        OutEvidence = {};
        return false;
    }

    return true;
}