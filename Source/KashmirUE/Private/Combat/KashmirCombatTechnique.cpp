#include "Combat/KashmirCombatTechnique.h"


bool FKashmirTechniqueRequest::IsValid(FString& OutReason) const
{
    OutReason.Reset();
    if (Slot == EKashmirTechniqueSlot::None)
    {
        OutReason = TEXT("technique request requires a logical slot");
        return false;
    }
    if (!FMath::IsFinite(DirectionToTarget.X) ||
        !FMath::IsFinite(DirectionToTarget.Y) ||
        !FMath::IsFinite(Intensity) || Intensity < 0.0f)
    {
        OutReason = TEXT("technique request contains invalid targeting evidence");
        return false;
    }
    return true;
}


bool FKashmirCombatTechniqueDefinition::IsValid(FString& OutReason) const
{
    OutReason.Reset();
    if (TechniqueId.IsNone() || ActionId.IsNone() || WeaponFamily.IsNone())
    {
        OutReason = TEXT("combat technique requires technique, action and weapon-family ids");
        return false;
    }
    if (AttackDirection == EKashmirAttackDirection::None)
    {
        OutReason = TEXT("combat technique requires authored attack direction");
        return false;
    }
    if (!FMath::IsFinite(PlayRate) || PlayRate <= 0.0f ||
        !FMath::IsFinite(BaseDamage) || BaseDamage < 0.0f ||
        !FMath::IsFinite(BaseGuardDamage) || BaseGuardDamage < 0.0f ||
        UltimateStage < 0)
    {
        OutReason = TEXT("combat technique contains invalid numeric configuration");
        return false;
    }
    if (bOverrideSwordPresentation && !SwordPresentation.IsValid(OutReason))
    {
        OutReason = FString::Printf(
            TEXT("combat technique has invalid sword presentation: %s"),
            *OutReason);
        return false;
    }
    if (RuntimeDefinition.ActionId != ActionId ||
        !RuntimeDefinition.IsValid(OutReason))
    {
        OutReason = FString::Printf(
            TEXT("combat technique has invalid runtime definition: %s"),
            *OutReason);
        return false;
    }
    if (CombatDefinition.Effects.IsEmpty())
    {
        OutReason = TEXT("combat technique requires at least one combat effect");
        return false;
    }
    return true;
}


FVector2D FKashmirCombatTechniqueDefinition::GetAuthoredDirectionVector() const
{
    switch (AttackDirection)
    {
    case EKashmirAttackDirection::LeftToRight:
        return FVector2D(1.0f, 0.0f);
    case EKashmirAttackDirection::RightToLeft:
        return FVector2D(-1.0f, 0.0f);
    case EKashmirAttackDirection::HighToLow:
        return FVector2D(0.0f, -1.0f);
    case EKashmirAttackDirection::LowToHigh:
        return FVector2D(0.0f, 1.0f);
    case EKashmirAttackDirection::DiagonalLeftToRight:
        return FVector2D(1.0f, -1.0f).GetSafeNormal();
    case EKashmirAttackDirection::DiagonalRightToLeft:
        return FVector2D(-1.0f, -1.0f).GetSafeNormal();
    case EKashmirAttackDirection::Thrust:
        return FVector2D(0.0f, 1.0f);
    case EKashmirAttackDirection::Rotational:
    case EKashmirAttackDirection::Radial:
        return FVector2D::UnitX();
    default:
        return FVector2D::ZeroVector;
    }
}


FKashmirActionRequest
FKashmirCombatTechniqueDefinition::BuildActionRequest(float Intensity) const
{
    FKashmirActionRequest Result;
    Result.ActionId = ActionId;
    Result.Direction = GetAuthoredDirectionVector();
    Result.Intensity = Intensity;
    Result.Curvature = AttackDirection == EKashmirAttackDirection::Rotational
        ? 1.0f
        : 0.0f;
    return Result;
}


bool UKashmirWeaponCombatStyle::ValidateStyle(FString& OutReason) const
{
    OutReason.Reset();
    if (StyleId.IsNone() || WeaponFamily.IsNone())
    {
        OutReason = TEXT("weapon combat style requires style and weapon-family ids");
        return false;
    }
    if (Techniques.IsEmpty() || SlotBindings.IsEmpty())
    {
        OutReason = TEXT("weapon combat style requires techniques and slot bindings");
        return false;
    }

    TSet<FName> TechniqueIds;
    for (const FKashmirCombatTechniqueDefinition& Technique : Techniques)
    {
        FString TechniqueReason;
        if (!Technique.IsValid(TechniqueReason))
        {
            OutReason = FString::Printf(
                TEXT("invalid technique '%s': %s"),
                *Technique.TechniqueId.ToString(), *TechniqueReason);
            return false;
        }
        if (Technique.WeaponFamily != WeaponFamily ||
            TechniqueIds.Contains(Technique.TechniqueId))
        {
            OutReason = TEXT("weapon combat style has mismatched or duplicate technique data");
            return false;
        }
        TechniqueIds.Add(Technique.TechniqueId);
    }

    TSet<EKashmirTechniqueSlot> BoundSlots;
    for (const FKashmirTechniqueSlotBinding& Binding : SlotBindings)
    {
        if (Binding.Slot == EKashmirTechniqueSlot::None ||
            Binding.TechniqueId.IsNone() ||
            !TechniqueIds.Contains(Binding.TechniqueId) ||
            BoundSlots.Contains(Binding.Slot))
        {
            OutReason = TEXT("weapon combat style has invalid or duplicate slot binding");
            return false;
        }
        BoundSlots.Add(Binding.Slot);
    }
    return true;
}


bool UKashmirWeaponCombatStyle::ResolveTechnique(
    const FKashmirTechniqueRequest& Request,
    FKashmirTechniqueActionPlan& OutPlan,
    FString& OutReason) const
{
    OutPlan = {};
    OutReason.Reset();
    if (!ValidateStyle(OutReason) || !Request.IsValid(OutReason))
    {
        return false;
    }

    const FKashmirTechniqueSlotBinding* Binding =
        SlotBindings.FindByPredicate(
            [&Request](const FKashmirTechniqueSlotBinding& Candidate)
            {
                return Candidate.Slot == Request.Slot;
            });
    if (Binding == nullptr)
    {
        OutReason = TEXT("requested technique slot is not bound by this style");
        return false;
    }

    const FKashmirCombatTechniqueDefinition* Technique =
        Techniques.FindByPredicate(
            [Binding](const FKashmirCombatTechniqueDefinition& Candidate)
            {
                return Candidate.TechniqueId == Binding->TechniqueId;
            });
    if (Technique == nullptr)
    {
        OutReason = TEXT("technique slot references a missing definition");
        return false;
    }

    OutPlan.StyleId = StyleId;
    OutPlan.TechniqueRequest = Request;
    OutPlan.Technique = *Technique;
    OutPlan.ActionRequest = Technique->BuildActionRequest(Request.Intensity);
    if (!OutPlan.ActionRequest.IsValid(OutReason))
    {
        OutPlan = {};
        return false;
    }
    OutPlan.bResolved = true;
    return true;
}
