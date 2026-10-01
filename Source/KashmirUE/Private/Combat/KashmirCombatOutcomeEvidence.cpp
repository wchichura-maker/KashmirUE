#include "Combat/KashmirCombatOutcomeEvidence.h"


namespace
{
    constexpr int32 AllOutcomeFactMask =
        static_cast<int32>(EKashmirCombatOutcomeFact::HadContact) |
        static_cast<int32>(EKashmirCombatOutcomeFact::AppliedDamage) |
        static_cast<int32>(EKashmirCombatOutcomeFact::Blocked) |
        static_cast<int32>(EKashmirCombatOutcomeFact::Parried) |
        static_cast<int32>(EKashmirCombatOutcomeFact::GuardBroken);

    bool HasRequiredFact(
        const int32 RequiredFacts,
        const EKashmirCombatOutcomeFact Fact)
    {
        return (RequiredFacts & static_cast<int32>(Fact)) != 0;
    }
}


FKashmirCombatOutcomeFacts FKashmirCombatOutcomeFacts::FromOutcome(
    const FKashmirCombatExecutionOutcome& Outcome)
{
    FKashmirCombatOutcomeFacts Facts;
    Facts.bHadContact = Outcome.bHadContact;
    Facts.bAppliedDamage = Outcome.bAppliedDamage;
    Facts.bBlocked = Outcome.bWasBlocked;
    Facts.bParried = Outcome.bWasParried;
    Facts.bGuardBroken = Outcome.bCausedGuardBreak;
    return Facts;
}


bool FKashmirCombatOutcomeFacts::IsValidRequirementMask(
    const int32 RequiredFacts)
{
    return RequiredFacts >= 0 && (RequiredFacts & ~AllOutcomeFactMask) == 0;
}


bool FKashmirCombatOutcomeFacts::Satisfies(const int32 RequiredFacts) const
{
    if (!IsValidRequirementMask(RequiredFacts))
    {
        return false;
    }
    return (!HasRequiredFact(RequiredFacts, EKashmirCombatOutcomeFact::HadContact) ||
            bHadContact) &&
        (!HasRequiredFact(RequiredFacts, EKashmirCombatOutcomeFact::AppliedDamage) ||
            bAppliedDamage) &&
        (!HasRequiredFact(RequiredFacts, EKashmirCombatOutcomeFact::Blocked) ||
            bBlocked) &&
        (!HasRequiredFact(RequiredFacts, EKashmirCombatOutcomeFact::Parried) ||
            bParried) &&
        (!HasRequiredFact(RequiredFacts, EKashmirCombatOutcomeFact::GuardBroken) ||
            bGuardBroken);
}


void FKashmirCombatOutcomeAccumulator::BeginExecution(
    const int64 ExecutionSerial,
    const FName TechniqueId,
    const FName ActionId)
{
    Current = {};
    Current.ExecutionSerial = ExecutionSerial;
    Current.TechniqueId = TechniqueId;
    Current.ActionId = ActionId;
    UniqueTargets.Reset();
    bHasCurrent = true;
}


bool FKashmirCombatOutcomeAccumulator::RecordContact(
    const FKashmirCombatOutcomeContact& Contact,
    FString& OutReason)
{
    OutReason.Reset();
    if (!bHasCurrent || Current.bFinalized)
    {
        OutReason = TEXT("combat outcome contact requires an active execution");
        return false;
    }
    if (Contact.TargetId.IsNone())
    {
        OutReason = TEXT("combat outcome contact requires target id");
        return false;
    }
    if (!FMath::IsFinite(Contact.DamageApplied) || Contact.DamageApplied < 0.0f)
    {
        OutReason = TEXT("combat outcome applied damage must be finite and non-negative");
        return false;
    }

    ++Current.ContactCount;
    Current.bHadContact = true;
    UniqueTargets.Add(Contact.TargetId);
    Current.UniqueTargetCount = UniqueTargets.Num();
    Current.bAppliedDamage |= Contact.bDamageApplied;
    Current.TotalDamageApplied += Contact.bDamageApplied
        ? Contact.DamageApplied
        : 0.0f;
    Current.bWasBlocked |= Contact.bBlocked;
    Current.bWasParried |= Contact.bParried;
    Current.bCausedGuardBreak |= Contact.bGuardBroken;
    Current.LastTargetId = Contact.TargetId;
    Current.LastCombatResult = Contact.CombatResult;
    return true;
}


void FKashmirCombatOutcomeAccumulator::Finalize(
    const EKashmirCombatOutcomeFinalizationReason Reason)
{
    if (!bHasCurrent)
    {
        return;
    }
    Current.bFinalized = true;
    Current.FinalizationReason = Reason;
    LastFinalized = Current;
    bHasLastFinalized = true;
    Current = {};
    UniqueTargets.Reset();
    bHasCurrent = false;
}


void FKashmirCombatOutcomeAccumulator::Reset()
{
    Current = {};
    LastFinalized = {};
    UniqueTargets.Reset();
    bHasCurrent = false;
    bHasLastFinalized = false;
}
