#include "Combat/KashmirClashResolver.h"


bool FKashmirClashInput::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    FString EvidenceReason;
    if (!FirstEvidence.IsValid(EvidenceReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid first hit evidence: %s"),
            *EvidenceReason);
        return false;
    }

    if (!SecondEvidence.IsValid(EvidenceReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid second hit evidence: %s"),
            *EvidenceReason);
        return false;
    }

    if (!FMath::IsFinite(MinimumRelativeSpeed) ||
        MinimumRelativeSpeed < 0.0f)
    {
        OutReason = TEXT("minimum clash relative speed must be finite and non-negative");
        return false;
    }

    return true;
}


bool FKashmirClashResolver::Resolve(
    const FKashmirClashInput& Input,
    FKashmirClashResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Input.IsValid(OutReason))
    {
        return false;
    }

    OutResult.RelativeVelocity =
        Input.FirstEvidence.ContactVelocity -
        Input.SecondEvidence.ContactVelocity;
    OutResult.RelativeSpeed = OutResult.RelativeVelocity.Size();

    if (!Input.bFirstAttackActive ||
        !Input.bSecondAttackActive ||
        Input.FirstEvidence.ContactSource != EKashmirContactSourceType::Weapon ||
        Input.SecondEvidence.ContactSource != EKashmirContactSourceType::Weapon ||
        Input.FirstEvidence.InstigatorId == Input.SecondEvidence.InstigatorId ||
        OutResult.RelativeSpeed < Input.MinimumRelativeSpeed)
    {
        return true;
    }

    OutResult.bClashed = true;
    OutResult.Outcome = EKashmirClashOutcome::Clash;
    return true;
}
