#include "Combat/KashmirParryResolver.h"


bool FKashmirParryWindow::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(StartTimeSeconds) ||
        !FMath::IsFinite(EndTimeSeconds))
    {
        OutReason = TEXT("parry window bounds must be finite");
        return false;
    }

    if (StartTimeSeconds < 0.0f || EndTimeSeconds < 0.0f)
    {
        OutReason = TEXT("parry window bounds must be non-negative");
        return false;
    }

    if (EndTimeSeconds < StartTimeSeconds)
    {
        OutReason = TEXT("parry window end must not precede start");
        return false;
    }

    return true;
}


bool FKashmirParryState::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(ElapsedTimeSeconds) ||
        ElapsedTimeSeconds < 0.0f)
    {
        OutReason = TEXT("parry elapsed time must be finite and non-negative");
        return false;
    }

    return Window.IsValid(OutReason);
}


bool FKashmirParryResolver::Resolve(
    const FKashmirHitEvidence& Evidence,
    const FKashmirBlockState& DefenseFacing,
    const FKashmirParryState& ParryState,
    FKashmirParryResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    FString ValidationReason;
    if (!Evidence.IsValid(ValidationReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid hit evidence: %s"),
            *ValidationReason);
        return false;
    }

    if (!DefenseFacing.IsValid(ValidationReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid defense facing: %s"),
            *ValidationReason);
        return false;
    }

    if (!ParryState.IsValid(ValidationReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid parry state: %s"),
            *ValidationReason);
        return false;
    }

    if (Evidence.AttackDirection.IsNearlyZero())
    {
        OutReason = TEXT("attack direction must not be zero");
        return false;
    }

    OutResult.ElapsedTimeSeconds = ParryState.ElapsedTimeSeconds;

    const FVector IncomingSourceDirection =
        -Evidence.AttackDirection.GetSafeNormal();

    OutResult.Alignment = FVector::DotProduct(
        DefenseFacing.ForwardDirection.GetSafeNormal(),
        IncomingSourceDirection);

    OutResult.RequiredAlignment = FMath::Cos(
        FMath::DegreesToRadians(DefenseFacing.HalfAngleDegrees));

    if (!ParryState.bActive)
    {
        OutResult.Timing = EKashmirParryTiming::Inactive;
        return true;
    }

    if (ParryState.ElapsedTimeSeconds < ParryState.Window.StartTimeSeconds)
    {
        OutResult.Timing = EKashmirParryTiming::TooEarly;
        return true;
    }

    if (ParryState.ElapsedTimeSeconds > ParryState.Window.EndTimeSeconds)
    {
        OutResult.Timing = EKashmirParryTiming::TooLate;
        return true;
    }

    if (ParryState.ElapsedTimeSeconds == ParryState.Window.StartTimeSeconds)
    {
        OutResult.Timing = EKashmirParryTiming::ExactStart;
    }
    else if (ParryState.ElapsedTimeSeconds == ParryState.Window.EndTimeSeconds)
    {
        OutResult.Timing = EKashmirParryTiming::ExactEnd;
    }
    else
    {
        OutResult.Timing = EKashmirParryTiming::InsideWindow;
    }

    OutResult.bParried =
        OutResult.Alignment >= OutResult.RequiredAlignment;

    return true;
}
