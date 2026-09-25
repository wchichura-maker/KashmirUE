#include "Combat/KashmirEffectApplier.h"

bool FKashmirHealthState::IsValid(
    FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(Current) ||
        !FMath::IsFinite(Maximum))
    {
        OutReason =
            TEXT("health values must be finite");

        return false;
    }

    if (Maximum < 0.0f)
    {
        OutReason =
            TEXT("maximum health must be non-negative");

        return false;
    }

    if (Current < 0.0f ||
        Current > Maximum)
    {
        OutReason =
            TEXT("current health must be within valid range");

        return false;
    }

    return true;
}

bool FKashmirEffectApplier::Apply(
    const FKashmirEffectResult& Effect,
    FKashmirHealthState& Health,
    FKashmirEffectApplicationResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Health.IsValid(
            OutReason))
    {
        return false;
    }

    if (!FMath::IsFinite(Effect.Magnitude) ||
        Effect.Magnitude < 0.0f)
    {
        OutReason =
            TEXT("effect magnitude must be non-negative");

        return false;
    }

    OutResult.HealthBefore =
        Health.Current;

    switch (Effect.Resolution)
    {
        case EKashmirResolutionType::Damage:
        {
            Health.Current =
                FMath::Max(
                    0.0f,
                    Health.Current -
                    Effect.Magnitude
                );

            break;
        }

        case EKashmirResolutionType::Heal:
        {
            Health.Current =
                FMath::Min(
                    Health.Maximum,
                    Health.Current +
                    Effect.Magnitude
                );

            break;
        }

        default:
        {
            OutReason =
                TEXT(
                    "effect resolution cannot modify health"
                );

            return false;
        }
    }

    OutResult.HealthAfter =
        Health.Current;

    OutResult.AppliedMagnitude =
        FMath::Abs(
            OutResult.HealthAfter -
            OutResult.HealthBefore
        );

    OutResult.bApplied = true;

    return true;
}