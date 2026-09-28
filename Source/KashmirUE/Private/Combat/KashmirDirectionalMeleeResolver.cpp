#include "Combat/KashmirDirectionalMeleeResolver.h"


bool FKashmirDirectionalMeleeResolver::Resolve(
    const FKashmirDirectionalSwordContact& Contact,
    const FKashmirDirectionalMeleeInput& Input,
    FKashmirDirectionalMeleeResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Contact.bResolved)
    {
        OutReason = TEXT("directional melee requires a resolved sword contact");
        return false;
    }
    if (Input.InstigatorId.IsNone() || Input.TargetId.IsNone() ||
        Input.SourceId.IsNone())
    {
        OutReason = TEXT("directional melee requires stable combat identities");
        return false;
    }
    if (Input.HitRegionMap == nullptr)
    {
        OutReason = TEXT("directional melee requires a hit region map");
        return false;
    }
    if (!FMath::IsFinite(Input.RegionMultiplier) ||
        Input.RegionMultiplier < 0.0f)
    {
        OutReason = TEXT("directional melee region multiplier must be finite and non-negative");
        return false;
    }

    FKashmirMeleeHitProcessInput HitInput;
    HitInput.InstigatorId = Input.InstigatorId;
    HitInput.TargetId = Input.TargetId;
    HitInput.SourceId = Input.SourceId;
    HitInput.ContactSource = Contact.ContactSource;
    HitInput.HitRegionMap = Input.HitRegionMap;
    HitInput.EvidenceTags = Input.EvidenceTags;
    HitInput.BaseDamage = Contact.BaseDamage;
    HitInput.AttackPowerMultiplier = Contact.AttackPowerMultiplier;
    HitInput.RegionMultiplier = Input.RegionMultiplier;

    FKashmirMeleeHitProcessResult HitResult;
    FKashmirMeleeHitProcessor HitProcessor;
    if (!HitProcessor.Process(
            Contact.TraceHit,
            Contact.CombatDefinition,
            HitInput,
            HitResult,
            OutReason))
    {
        return false;
    }

    FKashmirMeleeDefenseInput DefenseInput;
    DefenseInput.DefenseInput = Input.DefenseInput;
    DefenseInput.DefenseInput.BaseGuardDamage = Contact.BaseGuardDamage;

    FKashmirMeleeDefenseProcessor DefenseProcessor;
    if (!DefenseProcessor.Resolve(
            HitResult,
            DefenseInput,
            OutResult.Defense,
            OutReason))
    {
        OutResult = {};
        return false;
    }

    OutResult.bResolved = true;
    return true;
}
