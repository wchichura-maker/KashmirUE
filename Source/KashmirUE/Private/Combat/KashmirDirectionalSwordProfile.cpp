#include "Combat/KashmirDirectionalSwordProfile.h"


bool FKashmirSwordAuthoredAction::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (ActionId.IsNone())
    {
        OutReason = TEXT("authored sword action requires an action id");
        return false;
    }

    if (Montage.IsNull())
    {
        OutReason = TEXT("authored sword action requires a montage");
        return false;
    }

    if (!FMath::IsFinite(PlayRate) || PlayRate <= 0.0f)
    {
        OutReason = TEXT("authored sword action play rate must be finite and positive");
        return false;
    }

    if (!PoseConfig.IsValid(OutReason))
    {
        OutReason = FString::Printf(TEXT("invalid authored sword pose: %s"), *OutReason);
        return false;
    }

    if (CombatDefinition.Effects.IsEmpty())
    {
        OutReason = TEXT("authored sword action requires at least one combat effect");
        return false;
    }

    if (!FMath::IsFinite(BaseDamage) || BaseDamage < 0.0f)
    {
        OutReason = TEXT("authored sword action base damage must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(AttackPowerMultiplier) || AttackPowerMultiplier <= 0.0f)
    {
        OutReason = TEXT("authored sword attack power multiplier must be finite and positive");
        return false;
    }

    if (!FMath::IsFinite(BaseGuardDamage) || BaseGuardDamage < 0.0f)
    {
        OutReason = TEXT("authored sword guard damage must be finite and non-negative");
        return false;
    }

    const FKashmirActionDefinition RuntimeDefinition = BuildRuntimeDefinition();
    if (!RuntimeDefinition.IsValid(OutReason))
    {
        OutReason = FString::Printf(TEXT("invalid authored sword timeline: %s"), *OutReason);
        return false;
    }

    return true;
}


FKashmirActionDefinition FKashmirSwordAuthoredAction::BuildRuntimeDefinition() const
{
    FKashmirActionDefinition Result;
    Result.ActionId = ActionId;
    Result.StartupDuration = StartupDuration;
    Result.ActiveDuration = ActiveDuration;
    Result.RecoveryDuration = RecoveryDuration;
    Result.bCancellable = bCancellable;
    Result.CancelWindows = CancelWindows;
    Result.StartCosts = StartCosts;
    return Result;
}


bool UKashmirDirectionalSwordProfile::ValidateProfile(FString& OutReason) const
{
    OutReason.Reset();

    if (!ArchetypeDefinition.IsValid(OutReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid melee archetype: %s"), *OutReason);
        return false;
    }

    if (!GestureConfig.IsValid(OutReason))
    {
        OutReason = FString::Printf(TEXT("invalid directional sword gesture config: %s"), *OutReason);
        return false;
    }

    if (Actions.IsEmpty())
    {
        OutReason = TEXT("directional sword profile requires authored actions");
        return false;
    }

    TSet<FName> SeenActionIds;
    for (const FKashmirSwordAuthoredAction& Action : Actions)
    {
        FString ActionReason;
        if (!Action.IsValid(ActionReason))
        {
            OutReason = FString::Printf(
                TEXT("invalid authored sword action '%s': %s"),
                *Action.ActionId.ToString(),
                *ActionReason);
            return false;
        }

        if (SeenActionIds.Contains(Action.ActionId))
        {
            OutReason = FString::Printf(
                TEXT("duplicate authored sword action id '%s'"),
                *Action.ActionId.ToString());
            return false;
        }

        SeenActionIds.Add(Action.ActionId);
    }

    for (const FKashmirSwordActionBinding& Binding : GestureConfig.ActionBindings)
    {
        if (!SeenActionIds.Contains(Binding.ActionId))
        {
            OutReason = FString::Printf(
                TEXT("sword gesture binding references unknown action '%s'"),
                *Binding.ActionId.ToString());
            return false;
        }
    }

    return true;
}


const FKashmirSwordAuthoredAction*
UKashmirDirectionalSwordProfile::FindAction(FName ActionId) const
{
    return Actions.FindByPredicate(
        [ActionId](const FKashmirSwordAuthoredAction& Action)
        {
            return Action.ActionId == ActionId;
        });
}


bool UKashmirDirectionalSwordProfile::ResolveActionPlan(
    const FKashmirSwordGestureInput& Input,
    FKashmirSwordActionPlan& OutPlan,
    FString& OutReason) const
{
    OutPlan = {};
    OutReason.Reset();

    if (!ValidateProfile(OutReason))
    {
        return false;
    }

    FKashmirDirectionalSwordResolver Resolver;
    if (!Resolver.Resolve(Input, GestureConfig, OutPlan.Gesture, OutReason))
    {
        OutPlan = {};
        return false;
    }

    const FKashmirSwordAuthoredAction* Action =
        FindAction(OutPlan.Gesture.ActionRequest.ActionId);
    if (Action == nullptr)
    {
        OutReason = TEXT("resolved sword gesture has no authored action");
        OutPlan = {};
        return false;
    }

    OutPlan.RuntimeDefinition = Action->BuildRuntimeDefinition();
    OutPlan.ArchetypeDefinition = ArchetypeDefinition;
    OutPlan.Montage = Action->Montage;
    OutPlan.MontageSection = Action->MontageSection;
    OutPlan.PlayRate = Action->PlayRate;
    OutPlan.PoseConfig = Action->PoseConfig;
    OutPlan.CombatDefinition = Action->CombatDefinition;
    OutPlan.BaseDamage = Action->BaseDamage;
    OutPlan.AttackPowerMultiplier = Action->AttackPowerMultiplier;
    OutPlan.BaseGuardDamage = Action->BaseGuardDamage;
    OutPlan.bResolved = true;
    return true;
}


bool UKashmirDirectionalSwordProfile::ResolveActionPlan(
    const FKashmirActionRequest& Request,
    FKashmirSwordActionPlan& OutPlan,
    FString& OutReason) const
{
    OutPlan = {};
    OutReason.Reset();
    if (!ValidateProfile(OutReason) || !Request.IsValid(OutReason))
    {
        return false;
    }

    const FKashmirSwordAuthoredAction* Action = FindAction(Request.ActionId);
    const FKashmirSwordActionBinding* Binding =
        GestureConfig.ActionBindings.FindByPredicate(
            [&Request](const FKashmirSwordActionBinding& Candidate)
            {
                return Candidate.ActionId == Request.ActionId;
            });
    if (Action == nullptr || Binding == nullptr)
    {
        OutReason = TEXT("action request is not authored by this melee profile");
        return false;
    }

    OutPlan.Gesture.bGestureResolved = true;
    OutPlan.Gesture.Family = Binding->Family;
    OutPlan.Gesture.Direction = Binding->Direction;
    OutPlan.Gesture.DirectionVector = Request.Direction.GetSafeNormal();
    OutPlan.Gesture.Intensity = Request.Intensity;
    OutPlan.Gesture.Curvature = Request.Curvature;
    OutPlan.Gesture.ActionRequest = Request;
    OutPlan.ArchetypeDefinition = ArchetypeDefinition;
    OutPlan.RuntimeDefinition = Action->BuildRuntimeDefinition();
    OutPlan.Montage = Action->Montage;
    OutPlan.MontageSection = Action->MontageSection;
    OutPlan.PlayRate = Action->PlayRate;
    OutPlan.PoseConfig = Action->PoseConfig;
    OutPlan.CombatDefinition = Action->CombatDefinition;
    OutPlan.BaseDamage = Action->BaseDamage;
    OutPlan.AttackPowerMultiplier = Action->AttackPowerMultiplier;
    OutPlan.BaseGuardDamage = Action->BaseGuardDamage;
    OutPlan.bResolved = true;
    return true;
}
