#include "Combat/KashmirPhysicalReactionResolver.h"


bool FKashmirPhysicalReactionConfig::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(IntensityMultiplier) || IntensityMultiplier < 0.0f)
    {
        OutReason = TEXT("physical reaction intensity multiplier must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(MaximumIntensity) || MaximumIntensity < 0.0f)
    {
        OutReason = TEXT("maximum physical reaction intensity must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(PartialBodyBlendWeight) ||
        PartialBodyBlendWeight < 0.0f ||
        PartialBodyBlendWeight > 1.0f)
    {
        OutReason = TEXT("partial body blend weight must be between zero and one");
        return false;
    }

    if (!FMath::IsFinite(RecoveryDurationSeconds) || RecoveryDurationSeconds < 0.0f)
    {
        OutReason = TEXT("physical reaction recovery duration must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(RagdollCandidateThreshold) || RagdollCandidateThreshold < 0.0f)
    {
        OutReason = TEXT("ragdoll candidate threshold must be finite and non-negative");
        return false;
    }

    return true;
}


bool FKashmirPhysicalReactionInput::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    FString EvidenceReason;
    if (!Evidence.IsValid(EvidenceReason))
    {
        OutReason = FString::Printf(TEXT("invalid physical reaction evidence: %s"), *EvidenceReason);
        return false;
    }

    if (!Config.IsValid(OutReason))
    {
        return false;
    }

    if (!FMath::IsFinite(Stagger.PhysicalIntensity) || Stagger.PhysicalIntensity < 0.0f)
    {
        OutReason = TEXT("stagger physical intensity must be finite and non-negative");
        return false;
    }

    return true;
}


bool FKashmirPhysicalReactionResolver::Resolve(
    const FKashmirPhysicalReactionInput& Input,
    FKashmirPhysicalReactionResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Input.IsValid(OutReason))
    {
        return false;
    }

    if (!Input.Stagger.bStaggered)
    {
        return true;
    }

    if (Input.Evidence.AttackDirection.IsNearlyZero())
    {
        OutReason = TEXT("physical reaction requires a non-zero attack direction");
        return false;
    }

    const float ScaledIntensity =
        Input.Stagger.PhysicalIntensity * Input.Config.IntensityMultiplier;

    if (!FMath::IsFinite(ScaledIntensity))
    {
        OutReason = TEXT("resolved physical reaction intensity must be finite");
        return false;
    }

    OutResult.bReactionRequested = true;
    OutResult.Target = Input.Stagger.Target;
    OutResult.Direction =
        Input.Stagger.Target == EKashmirStaggerTarget::Attacker
            ? -Input.Evidence.AttackDirection.GetSafeNormal()
            : Input.Evidence.AttackDirection.GetSafeNormal();
    OutResult.Intensity = FMath::Min(ScaledIntensity, Input.Config.MaximumIntensity);
    OutResult.PartialBodyBlendWeight = Input.Config.PartialBodyBlendWeight;
    OutResult.RecoveryDurationSeconds = Input.Config.RecoveryDurationSeconds;

    if (Input.Stagger.Target == EKashmirStaggerTarget::Defender)
    {
        OutResult.HitRegion = Input.Evidence.HitRegion;
    }

    OutResult.Mode =
        Input.Config.bAllowRagdollCandidate &&
        OutResult.Intensity >= Input.Config.RagdollCandidateThreshold
            ? EKashmirPhysicalReactionMode::RagdollCandidate
            : EKashmirPhysicalReactionMode::PartialBody;

    return true;
}
