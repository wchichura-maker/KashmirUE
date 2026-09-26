#include "Combat/KashmirDefenseResolver.h"


bool FKashmirBlockState::IsValid(
    FString& OutReason) const
{
    OutReason.Reset();

    if (
        !FMath::IsFinite(ForwardDirection.X) ||
        !FMath::IsFinite(ForwardDirection.Y) ||
        !FMath::IsFinite(ForwardDirection.Z))
    {
        OutReason =
            TEXT(
                "block forward direction must be finite"
            );

        return false;
    }

    if (ForwardDirection.IsNearlyZero())
    {
        OutReason =
            TEXT(
                "block forward direction must not be zero"
            );

        return false;
    }

    if (!FMath::IsFinite(
            HalfAngleDegrees))
    {
        OutReason =
            TEXT(
                "block half angle must be finite"
            );

        return false;
    }

    if (
        HalfAngleDegrees < 0.0f ||
        HalfAngleDegrees > 180.0f)
    {
        OutReason =
            TEXT(
                "block half angle must be between 0 and 180 degrees"
            );

        return false;
    }

    return true;
}


bool FKashmirDefenseResolver::ResolveBlock(
    const FKashmirHitEvidence& Evidence,
    const FKashmirBlockState& BlockState,
    FKashmirDefenseResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    FString EvidenceReason;

    if (!Evidence.IsValid(
            EvidenceReason))
    {
        OutReason =
            FString::Printf(
                TEXT(
                    "invalid hit evidence: %s"
                ),
                *EvidenceReason
            );

        return false;
    }

    FString BlockReason;

    if (!BlockState.IsValid(
            BlockReason))
    {
        OutReason =
            FString::Printf(
                TEXT(
                    "invalid block state: %s"
                ),
                *BlockReason
            );

        return false;
    }

    if (Evidence.AttackDirection.IsNearlyZero())
    {
        OutReason =
            TEXT(
                "attack direction must not be zero"
            );

        return false;
    }

    const FVector DefenderForward =
        BlockState.ForwardDirection
            .GetSafeNormal();

    const FVector AttackTravelDirection =
        Evidence.AttackDirection
            .GetSafeNormal();

    /*
     * AttackDirection describes the direction
     * the strike is travelling.
     *
     * For geometric defense we need the
     * direction FROM defender TO attacker.
     *
     * Therefore:
     *
     * IncomingSourceDirection =
     *     -AttackTravelDirection
     */
    OutResult.IncomingSourceDirection =
        -AttackTravelDirection;

    OutResult.Alignment =
        FVector::DotProduct(
            DefenderForward,
            OutResult.IncomingSourceDirection
        );

    OutResult.RequiredAlignment =
        FMath::Cos(
            FMath::DegreesToRadians(
                BlockState.HalfAngleDegrees
            )
        );

    if (!BlockState.bActive)
    {
        OutResult.bBlocked =
            false;

        return true;
    }

    OutResult.bBlocked =
        OutResult.Alignment >=
        OutResult.RequiredAlignment;

    return true;
}