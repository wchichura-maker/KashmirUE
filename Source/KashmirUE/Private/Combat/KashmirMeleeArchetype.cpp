#include "Combat/KashmirMeleeArchetype.h"


FKashmirMeleeArchetypeDefinition::FKashmirMeleeArchetypeDefinition()
{
    ContactPointIds = {
        TEXT("Weapon_Base"),
        TEXT("Weapon_Mid"),
        TEXT("Weapon_Tip")
    };
}


bool FKashmirMeleeArchetypeDefinition::IsValid(FString& OutReason) const
{
    OutReason.Reset();
    if (StyleId.IsNone() || SourceId.IsNone())
    {
        OutReason = TEXT("melee archetype requires stable style and source ids");
        return false;
    }
    if (!FMath::IsFinite(DamageMultiplier) || DamageMultiplier < 0.0f ||
        !FMath::IsFinite(GuardDamageMultiplier) ||
        GuardDamageMultiplier < 0.0f ||
        !FMath::IsFinite(ReachScale) || ReachScale <= 0.0f)
    {
        OutReason = TEXT("melee archetype multipliers must be finite and non-negative, with positive reach");
        return false;
    }
    if (ContactPointIds.IsEmpty())
    {
        OutReason = TEXT("melee archetype requires semantic contact points");
        return false;
    }

    TSet<FName> UniquePoints;
    for (const FName PointId : ContactPointIds)
    {
        if (PointId.IsNone() || UniquePoints.Contains(PointId))
        {
            OutReason = TEXT("melee archetype contact points must be named and unique");
            return false;
        }
        UniquePoints.Add(PointId);
    }

    const bool bUnarmed = Archetype == EKashmirMeleeArchetype::Unarmed;
    if (bUnarmed &&
        ContactSource != EKashmirContactSourceType::Hand &&
        ContactSource != EKashmirContactSourceType::Foot)
    {
        OutReason = TEXT("unarmed archetype requires hand or foot contact evidence");
        return false;
    }
    if (!bUnarmed && ContactSource != EKashmirContactSourceType::Weapon)
    {
        OutReason = TEXT("equipped melee archetype requires weapon contact evidence");
        return false;
    }
    return true;
}


FKashmirMeleeArchetypeDefinition FKashmirMeleeArchetypeResolver::MakeBaseline(
    const EKashmirMeleeArchetype Archetype)
{
    FKashmirMeleeArchetypeDefinition Result;
    Result.Archetype = Archetype;
    Result.DamageMultiplier = 1.0f;
    Result.GuardDamageMultiplier = 1.0f;
    Result.ReachScale = 1.0f;

    switch (Archetype)
    {
    case EKashmirMeleeArchetype::OneHandedSword:
        Result.StyleId = TEXT("Melee.OneHandedSword");
        Result.SourceId = TEXT("Weapon.MainHand");
        break;
    case EKashmirMeleeArchetype::TwoHandedSword:
        Result.StyleId = TEXT("Melee.TwoHandedSword");
        Result.SourceId = TEXT("Weapon.TwoHandedSword");
        break;
    case EKashmirMeleeArchetype::Axe:
        Result.StyleId = TEXT("Melee.Axe");
        Result.SourceId = TEXT("Weapon.Axe");
        Result.ContactPointIds = { TEXT("Weapon_Grip"), TEXT("Weapon_Head") };
        break;
    case EKashmirMeleeArchetype::Spear:
        Result.StyleId = TEXT("Melee.Spear");
        Result.SourceId = TEXT("Weapon.Spear");
        Result.ContactPointIds = {
            TEXT("Weapon_Base"), TEXT("Weapon_Mid"), TEXT("Weapon_Tip")
        };
        break;
    case EKashmirMeleeArchetype::Unarmed:
        Result.StyleId = TEXT("Melee.Unarmed");
        Result.SourceId = TEXT("Body.Hands");
        Result.ContactSource = EKashmirContactSourceType::Hand;
        Result.ContactPointIds = { TEXT("Hand_R"), TEXT("Hand_L") };
        break;
    default:
        break;
    }
    return Result;
}


bool FKashmirMeleeArchetypeResolver::Resolve(
    const FKashmirMeleeArchetypeDefinition& Definition,
    const float BaseDamage,
    const float AttackPowerMultiplier,
    const float BaseGuardDamage,
    FKashmirMeleeArchetypeResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();
    if (!Definition.IsValid(OutReason))
    {
        return false;
    }
    if (!FMath::IsFinite(BaseDamage) || BaseDamage < 0.0f ||
        !FMath::IsFinite(AttackPowerMultiplier) ||
        AttackPowerMultiplier <= 0.0f ||
        !FMath::IsFinite(BaseGuardDamage) || BaseGuardDamage < 0.0f)
    {
        OutReason = TEXT("melee archetype requires valid authored combat values");
        return false;
    }

    OutResult.bResolved = true;
    OutResult.Archetype = Definition.Archetype;
    OutResult.StyleId = Definition.StyleId;
    OutResult.SourceId = Definition.SourceId;
    OutResult.ContactSource = Definition.ContactSource;
    OutResult.BaseDamage = BaseDamage * Definition.DamageMultiplier;
    OutResult.AttackPowerMultiplier = AttackPowerMultiplier;
    OutResult.BaseGuardDamage =
        BaseGuardDamage * Definition.GuardDamageMultiplier;
    return true;
}
