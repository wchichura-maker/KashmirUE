#include "Combat/KashmirCombatTechnique.h"


namespace
{
    bool AreTechniqueTransitionRulesEquivalent(
        const FKashmirTechniqueTransitionRule& A,
        const FKashmirTechniqueTransitionRule& B)
    {
        return A.FromTechniqueId == B.FromTechniqueId &&
            A.ToTechniqueId == B.ToTechniqueId &&
            FMath::IsNearlyEqual(A.MinElapsed, B.MinElapsed) &&
            FMath::IsNearlyEqual(A.MaxElapsed, B.MaxElapsed) &&
            A.Priority == B.Priority &&
            A.RequiredTags == B.RequiredTags &&
            A.BlockedTags == B.BlockedTags &&
            A.RequiredOutcomeFacts == B.RequiredOutcomeFacts;
    }

    bool IsTechniqueTransitionRuleEligible(
        const FKashmirTechniqueTransitionRule& Rule,
        const FName FromTechniqueId,
        const FKashmirTechniqueTransitionContext& Context)
    {
        FString Reason;
        return Rule.IsValid(Reason) &&
            Rule.FromTechniqueId == FromTechniqueId &&
            Context.Elapsed + KINDA_SMALL_NUMBER >= Rule.MinElapsed &&
            (Rule.MaxElapsed < 0.0f ||
                Context.Elapsed - KINDA_SMALL_NUMBER <= Rule.MaxElapsed) &&
            Context.ContextTags.HasAll(Rule.RequiredTags) &&
            !Context.ContextTags.HasAny(Rule.BlockedTags) &&
            Context.OutcomeFacts.Satisfies(Rule.RequiredOutcomeFacts);
    }
}


bool FKashmirTechniqueMovementSpec::IsValid(FString& OutReason) const
{
    OutReason.Reset();
    if (!FMath::IsFinite(Distance) || Distance < 0.0f ||
        !FMath::IsFinite(Duration) || Duration < 0.0f)
    {
        OutReason = TEXT("technique movement spec contains invalid numeric configuration");
        return false;
    }
    if (Delivery == EKashmirMovementDelivery::None)
    {
        if (!FMath::IsNearlyZero(Distance) || !FMath::IsNearlyZero(Duration))
        {
            OutReason = TEXT("movement delivery None requires zero distance and duration");
            return false;
        }
        return true;
    }
    if (Distance <= UE_SMALL_NUMBER || Duration <= UE_SMALL_NUMBER)
    {
        OutReason = TEXT("controlled translation requires positive distance and duration");
        return false;
    }
    if (Reference == EKashmirMovementReference::Actor &&
        TargetPolicy != EKashmirMovementTargetPolicy::NotRequired)
    {
        OutReason = TEXT("actor-relative movement cannot require a target");
        return false;
    }
    if (Reference == EKashmirMovementReference::Target)
    {
        if (TargetPolicy == EKashmirMovementTargetPolicy::NotRequired)
        {
            OutReason = TEXT("target-relative movement requires an explicit target policy");
            return false;
        }
        OutReason = TEXT("target-relative movement is defined but not executable in v0.1");
        return false;
    }
    if (Direction != EKashmirMovementDirection::Forward)
    {
        OutReason = TEXT("movement direction is defined but not executable in v0.1");
        return false;
    }
    return true;
}


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


bool FKashmirTechniqueTransitionRule::IsValid(FString& OutReason) const
{
    OutReason.Reset();
    if (FromTechniqueId.IsNone() || ToTechniqueId.IsNone())
    {
        OutReason = TEXT("technique transition requires source and destination ids");
        return false;
    }
    if (FromTechniqueId == ToTechniqueId)
    {
        OutReason = TEXT("technique self-transition is not supported in v0.1");
        return false;
    }
    if (!FMath::IsFinite(MinElapsed) || !FMath::IsFinite(MaxElapsed) ||
        MinElapsed < 0.0f ||
        (MaxElapsed >= 0.0f && MaxElapsed < MinElapsed))
    {
        OutReason = TEXT("technique transition window is invalid");
        return false;
    }
    if (RequiredTags.HasAny(BlockedTags))
    {
        OutReason = TEXT("technique transition requires and blocks the same tag");
        return false;
    }
    if (!FKashmirCombatOutcomeFacts::IsValidRequirementMask(
            RequiredOutcomeFacts))
    {
        OutReason = TEXT("technique transition has invalid outcome requirements");
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
    if (!MovementSpec.IsValid(OutReason))
    {
        OutReason = FString::Printf(
            TEXT("combat technique has invalid movement spec: %s"),
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

    for (int32 RuleIndex = 0; RuleIndex < TransitionRules.Num(); ++RuleIndex)
    {
        const FKashmirTechniqueTransitionRule& Rule = TransitionRules[RuleIndex];
        FString RuleReason;
        if (!Rule.IsValid(RuleReason))
        {
            OutReason = FString::Printf(
                TEXT("invalid technique transition rule %d: %s"),
                RuleIndex,
                *RuleReason);
            return false;
        }
        if (!TechniqueIds.Contains(Rule.FromTechniqueId))
        {
            OutReason = FString::Printf(
                TEXT("technique transition references missing source '%s'"),
                *Rule.FromTechniqueId.ToString());
            return false;
        }
        if (!TechniqueIds.Contains(Rule.ToTechniqueId))
        {
            OutReason = FString::Printf(
                TEXT("technique transition references missing destination '%s'"),
                *Rule.ToTechniqueId.ToString());
            return false;
        }
        for (int32 PreviousIndex = 0; PreviousIndex < RuleIndex; ++PreviousIndex)
        {
            if (AreTechniqueTransitionRulesEquivalent(
                    Rule, TransitionRules[PreviousIndex]))
            {
                OutReason = TEXT("weapon combat style has a duplicate technique transition rule");
                return false;
            }
        }
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


TArray<FName> UKashmirWeaponCombatStyle::GetTechniqueTransitionOptions(
    const FName FromTechniqueId,
    const float Elapsed,
    const FGameplayTagContainer& ContextTags) const
{
    FKashmirTechniqueTransitionContext Context;
    Context.Elapsed = Elapsed;
    Context.ContextTags = ContextTags;
    return GetTechniqueTransitionOptions(FromTechniqueId, Context);
}


TArray<FName> UKashmirWeaponCombatStyle::GetTechniqueTransitionOptions(
    const FName FromTechniqueId,
    const FKashmirTechniqueTransitionContext& Context) const
{
    struct FCandidate
    {
        FName TechniqueId;
        int32 Priority = 0;
    };

    if (!FMath::IsFinite(Context.Elapsed) || Context.Elapsed < 0.0f)
    {
        return {};
    }

    TMap<FName, int32> BestPriorityByDestination;
    for (const FKashmirTechniqueTransitionRule& Rule : TransitionRules)
    {
        if (IsTechniqueTransitionRuleEligible(
                Rule, FromTechniqueId, Context))
        {
            int32& BestPriority = BestPriorityByDestination.FindOrAdd(
                Rule.ToTechniqueId, MIN_int32);
            BestPriority = FMath::Max(BestPriority, Rule.Priority);
        }
    }

    TArray<FCandidate> Candidates;
    for (const TPair<FName, int32>& Pair : BestPriorityByDestination)
    {
        Candidates.Add({Pair.Key, Pair.Value});
    }
    Candidates.Sort([](const FCandidate& A, const FCandidate& B)
    {
        return A.Priority != B.Priority
            ? A.Priority > B.Priority
            : A.TechniqueId.LexicalLess(B.TechniqueId);
    });

    TArray<FName> Result;
    Result.Reserve(Candidates.Num());
    for (const FCandidate& Candidate : Candidates)
    {
        Result.Add(Candidate.TechniqueId);
    }
    return Result;
}


bool UKashmirWeaponCombatStyle::ResolveTechniqueTransition(
    const FName FromTechniqueId,
    const FName ToTechniqueId,
    const float Elapsed,
    const FGameplayTagContainer& ContextTags,
    FKashmirTechniqueTransitionRule& OutRule,
    FString& OutReason) const
{
    FKashmirTechniqueTransitionContext Context;
    Context.Elapsed = Elapsed;
    Context.ContextTags = ContextTags;
    return ResolveTechniqueTransition(
        FromTechniqueId, ToTechniqueId, Context, OutRule, OutReason);
}


bool UKashmirWeaponCombatStyle::ResolveTechniqueTransition(
    const FName FromTechniqueId,
    const FName ToTechniqueId,
    const FKashmirTechniqueTransitionContext& Context,
    FKashmirTechniqueTransitionRule& OutRule,
    FString& OutReason) const
{
    OutRule = {};
    OutReason.Reset();
    if (!ValidateStyle(OutReason))
    {
        return false;
    }
    if (!FMath::IsFinite(Context.Elapsed) || Context.Elapsed < 0.0f)
    {
        OutReason = TEXT("technique transition elapsed time is invalid");
        return false;
    }

    const FKashmirTechniqueTransitionRule* BestRule = nullptr;
    for (const FKashmirTechniqueTransitionRule& Rule : TransitionRules)
    {
        if (Rule.ToTechniqueId != ToTechniqueId ||
            !IsTechniqueTransitionRuleEligible(
                Rule, FromTechniqueId, Context))
        {
            continue;
        }
        if (BestRule == nullptr || Rule.Priority > BestRule->Priority)
        {
            BestRule = &Rule;
        }
    }
    if (BestRule == nullptr)
    {
        OutReason = TEXT("technique transition is not available");
        return false;
    }
    OutRule = *BestRule;
    return true;
}


bool UKashmirWeaponCombatStyle::EvaluateTechniqueTransition(
    const FName FromTechniqueId,
    const FName ToTechniqueId,
    const FKashmirTechniqueTransitionContext& Context,
    const float MaximumFutureWait,
    const float SourceActionLifetime,
    FKashmirTechniqueTransitionEvaluation& OutEvaluation,
    FString& OutReason) const
{
    OutEvaluation = {};
    OutReason.Reset();
    if (!ValidateStyle(OutReason))
    {
        return false;
    }
    if (!FMath::IsFinite(Context.Elapsed) || Context.Elapsed < 0.0f ||
        !FMath::IsFinite(MaximumFutureWait) || MaximumFutureWait < 0.0f ||
        !FMath::IsFinite(SourceActionLifetime) || SourceActionLifetime < 0.0f)
    {
        OutReason = TEXT("technique transition evaluation contains invalid time");
        return false;
    }

    const FKashmirTechniqueTransitionRule* BestEligible = nullptr;
    const FKashmirTechniqueTransitionRule* BestPending = nullptr;
    const FKashmirTechniqueTransitionRule* BestFuture = nullptr;
    bool bWindowMissed = false;
    for (const FKashmirTechniqueTransitionRule& Rule : TransitionRules)
    {
        FString RuleReason;
        if (!Rule.IsValid(RuleReason) ||
            Rule.FromTechniqueId != FromTechniqueId ||
            Rule.ToTechniqueId != ToTechniqueId ||
            !Context.ContextTags.HasAll(Rule.RequiredTags) ||
            Context.ContextTags.HasAny(Rule.BlockedTags))
        {
            continue;
        }

        if (Rule.MaxElapsed >= 0.0f &&
            Context.Elapsed - KINDA_SMALL_NUMBER > Rule.MaxElapsed)
        {
            bWindowMissed = true;
            continue;
        }

        if (Context.Elapsed + KINDA_SMALL_NUMBER >= Rule.MinElapsed)
        {
            if (Context.OutcomeFacts.Satisfies(Rule.RequiredOutcomeFacts))
            {
                if (BestEligible == nullptr || Rule.Priority > BestEligible->Priority)
                {
                    BestEligible = &Rule;
                }
            }
            else if (BestPending == nullptr || Rule.Priority > BestPending->Priority)
            {
                BestPending = &Rule;
            }
            continue;
        }

        const float Wait = Rule.MinElapsed - Context.Elapsed;
        if (Rule.MinElapsed - KINDA_SMALL_NUMBER <= SourceActionLifetime &&
            Wait - KINDA_SMALL_NUMBER <= MaximumFutureWait &&
            (BestFuture == nullptr ||
                Rule.MinElapsed < BestFuture->MinElapsed ||
                (FMath::IsNearlyEqual(Rule.MinElapsed, BestFuture->MinElapsed) &&
                    Rule.Priority > BestFuture->Priority)))
        {
            BestFuture = &Rule;
        }
    }

    if (BestEligible != nullptr)
    {
        OutEvaluation.Availability =
            EKashmirTechniqueTransitionAvailability::EligibleNow;
        OutEvaluation.Rule = *BestEligible;
    }
    else if (BestPending != nullptr)
    {
        OutEvaluation.Availability =
            EKashmirTechniqueTransitionAvailability::OutcomePending;
        OutEvaluation.Rule = *BestPending;
    }
    else if (BestFuture != nullptr)
    {
        OutEvaluation.Availability =
            EKashmirTechniqueTransitionAvailability::FutureWindowReachable;
        OutEvaluation.Rule = *BestFuture;
    }
    else if (bWindowMissed)
    {
        OutEvaluation.Availability =
            EKashmirTechniqueTransitionAvailability::WindowMissed;
    }
    else
    {
        OutEvaluation.Availability =
            EKashmirTechniqueTransitionAvailability::NoMatchingRule;
    }
    return true;
}
