#include "Combat/KashmirDirectionalSwordComponent.h"

#include "Combat/KashmirSwordPresentationComponent.h"
#include "Combat/KashmirMovementDeliveryComponent.h"


bool FKashmirDirectionalSwordContactResolver::Resolve(
    const FKashmirWeaponTraceHit& TraceHit,
    const FKashmirSwordActionPlan& Plan,
    FKashmirDirectionalSwordContact& OutContact,
    FString& OutReason) const
{
    OutContact = {};
    OutReason.Reset();

    if (!Plan.bResolved || !Plan.Gesture.bGestureResolved)
    {
        OutReason = TEXT("directional sword contact requires a resolved action plan");
        return false;
    }

    if (!Plan.Gesture.ActionRequest.IsValid(OutReason))
    {
        OutReason = FString::Printf(
            TEXT("invalid directional sword contact action request: %s"),
            *OutReason);
        return false;
    }

    if (!FMath::IsFinite(TraceHit.Speed) || TraceHit.Speed < 0.0f ||
        TraceHit.ContactVelocity.ContainsNaN() ||
        TraceHit.AttackDirection.ContainsNaN() ||
        TraceHit.Hit.ImpactPoint.ContainsNaN() ||
        TraceHit.Hit.ImpactNormal.ContainsNaN())
    {
        OutReason = TEXT("directional sword trace hit contains invalid physical evidence");
        return false;
    }

    if (Plan.CombatDefinition.Effects.IsEmpty())
    {
        OutReason = TEXT("directional sword contact requires valid authored combat data");
        return false;
    }

    FKashmirMeleeArchetypeResult ArchetypeResult;
    FKashmirMeleeArchetypeResolver ArchetypeResolver;
    if (!ArchetypeResolver.Resolve(
            Plan.ArchetypeDefinition,
            Plan.BaseDamage,
            Plan.AttackPowerMultiplier,
            Plan.BaseGuardDamage,
            ArchetypeResult,
            OutReason))
    {
        return false;
    }

    OutContact.bResolved = true;
    OutContact.TraceHit = TraceHit;
    OutContact.ActionId = Plan.RuntimeDefinition.ActionId;
    OutContact.ActionRequest = Plan.Gesture.ActionRequest;
    OutContact.CombatDefinition = Plan.CombatDefinition;
    OutContact.Archetype = ArchetypeResult.Archetype;
    OutContact.StyleId = ArchetypeResult.StyleId;
    OutContact.SourceId = ArchetypeResult.SourceId;
    OutContact.ContactSource = ArchetypeResult.ContactSource;
    OutContact.BaseDamage = ArchetypeResult.BaseDamage;
    OutContact.AttackPowerMultiplier =
        ArchetypeResult.AttackPowerMultiplier;
    OutContact.BaseGuardDamage = ArchetypeResult.BaseGuardDamage;
    return true;
}


UKashmirDirectionalSwordComponent::UKashmirDirectionalSwordComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}


void UKashmirDirectionalSwordComponent::BeginPlay()
{
    Super::BeginPlay();

    FString Reason;
    RebuildRuntime(Reason);
}


void UKashmirDirectionalSwordComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    FinalizeCombatOutcome(EKashmirCombatOutcomeFinalizationReason::Interrupted);
    ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::SourceEnded);
    Super::EndPlay(EndPlayReason);
}


void UKashmirDirectionalSwordComponent::SetProfile(
    UKashmirDirectionalSwordProfile* InProfile)
{
    FinalizeCombatOutcome(EKashmirCombatOutcomeFinalizationReason::Interrupted);
    ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::SourceChanged);
    if (WeaponTraceComponent != nullptr &&
        WeaponTraceComponent->IsTraceWindowActive())
    {
        WeaponTraceComponent->EndTraceWindow();
    }
    Profile = InProfile;
    if (MovementDeliveryComponent != nullptr)
    {
        MovementDeliveryComponent->CancelDelivery();
    }
    CancelGesture();
    ActivePlan = {};
    ++ActionExecutionSerial;
    Runtime.Reset();
    Resources.Reset();

    FString Reason;
    RebuildRuntime(Reason);
}


void UKashmirDirectionalSwordComponent::SetPresentationComponent(
    UKashmirSwordPresentationComponent* InPresentationComponent)
{
    PresentationComponent = InPresentationComponent;
}


void UKashmirDirectionalSwordComponent::SetWeaponTraceComponent(
    UKashmirWeaponTraceComponent* InWeaponTraceComponent)
{
    if (WeaponTraceComponent != nullptr &&
        WeaponTraceComponent->IsTraceWindowActive())
    {
        WeaponTraceComponent->EndTraceWindow();
    }

    WeaponTraceComponent = InWeaponTraceComponent;
    const FKashmirActionRuntimeState State = GetRuntimeState();
    if (WeaponTraceComponent != nullptr &&
        State.Phase == EKashmirActionPhase::Active)
    {
        WeaponTraceComponent->BeginTraceWindow();
    }
}


void UKashmirDirectionalSwordComponent::SetMovementDeliveryComponent(
    UKashmirMovementDeliveryComponent* InMovementDeliveryComponent)
{
    if (MovementDeliveryComponent != nullptr &&
        MovementDeliveryComponent != InMovementDeliveryComponent)
    {
        MovementDeliveryComponent->CancelDelivery();
    }
    MovementDeliveryComponent = InMovementDeliveryComponent;
}


bool UKashmirDirectionalSwordComponent::RebuildRuntime(FString& OutReason)
{
    OutReason.Reset();
    ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::SourceChanged);
    Runtime.Reset();
    Resources.Reset();

    if (Profile == nullptr)
    {
        OutReason = TEXT("directional sword component requires a profile");
        return false;
    }

    if (!Profile->ValidateProfile(OutReason))
    {
        return false;
    }

    TMap<FName, FKashmirActionDefinition> Definitions;
    for (const FKashmirSwordAuthoredAction& Action : Profile->Actions)
    {
        Definitions.Add(Action.ActionId, Action.BuildRuntimeDefinition());
    }

    Resources = MakeUnique<FKashmirResourceRuntime>(InitialResources);
    Runtime = MakeUnique<FKashmirActionRuntime>(
        Definitions,
        TArray<FKashmirTransitionRule>{},
        *Resources);
    return true;
}


bool UKashmirDirectionalSwordComponent::BeginGesture(FString& OutReason)
{
    OutReason.Reset();
    if (Profile == nullptr)
    {
        OutReason = TEXT("directional sword gesture requires a profile");
        return false;
    }

    CapturedGesture = {};
    AccumulatedPosition = FVector2D::ZeroVector;
    CapturedGesture.Samples.Add(AccumulatedPosition);
    bCapturingGesture = true;
    return true;
}


bool UKashmirDirectionalSwordComponent::AddGestureDelta(
    FVector2D Delta,
    float DeltaSeconds,
    FString& OutReason)
{
    OutReason.Reset();
    if (!bCapturingGesture)
    {
        OutReason = TEXT("directional sword gesture is not being captured");
        return false;
    }

    if (!FMath::IsFinite(Delta.X) ||
        !FMath::IsFinite(Delta.Y) ||
        !FMath::IsFinite(DeltaSeconds) ||
        DeltaSeconds < 0.0f)
    {
        OutReason = TEXT("directional sword gesture delta must be finite and non-negative");
        return false;
    }

    CapturedGesture.DurationSeconds += DeltaSeconds;
    if (!Delta.IsNearlyZero())
    {
        AccumulatedPosition += Delta;
        CapturedGesture.Samples.Add(AccumulatedPosition);
    }
    return true;
}


bool UKashmirDirectionalSwordComponent::CompleteGesture(FString& OutReason)
{
    OutReason.Reset();
    if (!bCapturingGesture)
    {
        OutReason = TEXT("directional sword gesture is not being captured");
        return false;
    }
    bCapturingGesture = false;

    FKashmirSwordActionPlan Plan;
    if (!Profile->ResolveActionPlan(CapturedGesture, Plan, OutReason))
    {
        const float DirectDistance = CapturedGesture.Samples.Num() >= 2
            ? (CapturedGesture.Samples.Last() - CapturedGesture.Samples[0]).Size()
            : 0.0f;
        OutReason = FString::Printf(
            TEXT("%s (samples=%d duration=%.3fs direct_distance=%.2f)"),
            *OutReason,
            CapturedGesture.Samples.Num(),
            CapturedGesture.DurationSeconds,
            DirectDistance);
        return false;
    }

    return StartResolvedPlan(Plan, OutReason);
}


bool UKashmirDirectionalSwordComponent::StartActionRequest(
    const FKashmirActionRequest& Request,
    FString& OutReason)
{
    OutReason.Reset();
    if (Profile == nullptr)
    {
        OutReason = TEXT("action request requires a melee profile");
        return false;
    }

    FKashmirSwordActionPlan Plan;
    if (!Profile->ResolveActionPlan(Request, Plan, OutReason))
    {
        return false;
    }
    return StartResolvedPlan(Plan, OutReason);
}


bool UKashmirDirectionalSwordComponent::ResolveTechniqueRequestPlan(
    const FKashmirTechniqueRequest& Request,
    const UKashmirWeaponCombatStyle* Style,
    FKashmirTechniqueActionPlan& OutTechniquePlan,
    FKashmirSwordActionPlan& OutPlan,
    FString& OutReason) const
{
    OutReason.Reset();
    OutTechniquePlan = {};
    OutPlan = {};
    if (Style == nullptr)
    {
        OutReason = TEXT("technique request requires a weapon combat style");
        return false;
    }

    if (!Style->ResolveTechnique(Request, OutTechniquePlan, OutReason))
    {
        return false;
    }

    // The existing profile remains the runtime/combat adapter during migration.
    // Technique data owns Base Motion and may override only presentation; it
    // never replaces trace, damage, hit evidence or ActionRuntime authority.
    if (Profile == nullptr ||
        !Profile->ResolveActionPlan(
            OutTechniquePlan.ActionRequest,
            OutPlan,
            OutReason))
    {
        return false;
    }

    OutPlan.Montage = OutTechniquePlan.Technique.Montage;
    OutPlan.MontageSection = OutTechniquePlan.Technique.MontageSection;
    OutPlan.PlayRate = OutTechniquePlan.Technique.PlayRate;
    OutPlan.TechniqueId = OutTechniquePlan.Technique.TechniqueId;
    OutPlan.StyleId = OutTechniquePlan.StyleId;
    OutPlan.MovementIntent = OutTechniquePlan.Technique.MovementIntent;
    OutPlan.MovementSpec = OutTechniquePlan.Technique.MovementSpec;
    if (OutTechniquePlan.Technique.bOverrideSwordPresentation)
    {
        OutPlan.PoseConfig = OutTechniquePlan.Technique.SwordPresentation;
    }
    return true;
}


bool UKashmirDirectionalSwordComponent::StartTechniqueRequest(
    const FKashmirTechniqueRequest& Request,
    const UKashmirWeaponCombatStyle* Style,
    FString& OutReason)
{
    OutReason.Reset();
    LastTechniqueRequestReason.Reset();

    FKashmirTechniqueActionPlan TechniquePlan;
    FKashmirSwordActionPlan Plan;
    if (!ResolveTechniqueRequestPlan(
            Request, Style, TechniquePlan, Plan, OutReason))
    {
        LastTechniqueRequestReason = OutReason;
        return false;
    }

    const FKashmirActionRuntimeState CurrentState = GetRuntimeState();
    const FKashmirTechniqueTransitionContext TransitionContext =
        BuildTechniqueTransitionContext(CurrentState, Request.ContextTags);
    FKashmirTechniqueTransitionRule TransitionRule;
    FString TransitionReason;
    const bool bHasEligibleTransition =
        CurrentState.bActive &&
        ActivePlan.bResolved &&
        !ActivePlan.TechniqueId.IsNone() &&
        ActivePlan.StyleId == TechniquePlan.StyleId &&
        Style->ResolveTechniqueTransition(
            ActivePlan.TechniqueId,
            TechniquePlan.Technique.TechniqueId,
            TransitionContext,
            TransitionRule,
            TransitionReason);

    bool bStarted = false;
    if (bHasEligibleTransition)
    {
        bStarted = TryTransitionTechnique(
            Plan, TransitionRule, Request.ContextTags, OutReason);
        if (bStarted)
        {
            ClearPendingTechniqueRequest(
                EKashmirPendingTechniqueClearReason::SourceChanged);
        }
    }
    else
    {
        const ETechniqueBufferAttempt BufferAttempt = TryBufferTechniqueRequest(
            Request, Style, TechniquePlan, CurrentState, OutReason);
        if (BufferAttempt == ETechniqueBufferAttempt::Buffered)
        {
            return true;
        }
        if (BufferAttempt == ETechniqueBufferAttempt::Rejected)
        {
            LastTechniqueRequestReason = OutReason;
            return false;
        }
        bStarted = StartResolvedPlan(Plan, OutReason);
    }
    if (!bStarted)
    {
        LastTechniqueRequestReason = OutReason;
    }
    return bStarted;
}


FKashmirTechniqueTransitionContext
UKashmirDirectionalSwordComponent::BuildTechniqueTransitionContext(
    const FKashmirActionRuntimeState& RuntimeState,
    const FGameplayTagContainer& ContextTags) const
{
    FKashmirTechniqueTransitionContext Context;
    Context.Elapsed = RuntimeState.Elapsed;
    Context.ContextTags = ContextTags;
    if (CombatOutcome.HasCurrent())
    {
        const FKashmirCombatExecutionOutcome& Outcome = CombatOutcome.GetCurrent();
        if (Outcome.ExecutionSerial == static_cast<int64>(ActionExecutionSerial) &&
            Outcome.TechniqueId == ActivePlan.TechniqueId &&
            Outcome.ActionId == RuntimeState.ActionId)
        {
            Context.OutcomeFacts =
                FKashmirCombatOutcomeFacts::FromOutcome(Outcome);
        }
    }
    return Context;
}


UKashmirDirectionalSwordComponent::ETechniqueBufferAttempt
UKashmirDirectionalSwordComponent::TryBufferTechniqueRequest(
    const FKashmirTechniqueRequest& Request,
    const UKashmirWeaponCombatStyle* Style,
    const FKashmirTechniqueActionPlan& TechniquePlan,
    const FKashmirActionRuntimeState& CurrentState,
    FString& OutReason)
{
    if (!CurrentState.bActive || !ActivePlan.bResolved ||
        ActivePlan.TechniqueId.IsNone() ||
        ActivePlan.StyleId != TechniquePlan.StyleId || Runtime == nullptr)
    {
        return ETechniqueBufferAttempt::NotApplicable;
    }

    float SourceActionLifetime = 0.0f;
    FString LifetimeReason;
    if (!Runtime->GetActionTotalDuration(
            CurrentState.ActionId, SourceActionLifetime, LifetimeReason))
    {
        OutReason = LifetimeReason;
        return ETechniqueBufferAttempt::Rejected;
    }

    const float Lifetime = FMath::Max(0.0f, TechniqueRequestBufferLifetime);
    const FKashmirTechniqueTransitionContext Context =
        BuildTechniqueTransitionContext(CurrentState, Request.ContextTags);
    FKashmirTechniqueTransitionEvaluation Evaluation;
    if (!Style->EvaluateTechniqueTransition(
            ActivePlan.TechniqueId,
            TechniquePlan.Technique.TechniqueId,
            Context,
            Lifetime,
            SourceActionLifetime,
            Evaluation,
            OutReason))
    {
        return ETechniqueBufferAttempt::Rejected;
    }
    if (Evaluation.Availability ==
            EKashmirTechniqueTransitionAvailability::FutureWindowReachable ||
        Evaluation.Availability ==
            EKashmirTechniqueTransitionAvailability::OutcomePending)
    {
        if (PendingTechniqueRequest.IsSet())
        {
            ClearPendingTechniqueRequest(
                EKashmirPendingTechniqueClearReason::Superseded);
        }
        else
        {
            LastPendingTechniqueClearReason =
                EKashmirPendingTechniqueClearReason::None;
        }

        FKashmirPendingTechniqueRequestState Pending;
        Pending.Request = Request;
        Pending.Style = Style;
        Pending.TechniqueId = TechniquePlan.Technique.TechniqueId;
        Pending.SourceActionId = CurrentState.ActionId;
        Pending.SourceTechniqueId = ActivePlan.TechniqueId;
        Pending.SourceExecutionSerial = ActionExecutionSerial;
        Pending.BufferedAtActionElapsed = CurrentState.Elapsed;
        Pending.Lifetime = Lifetime;
        PendingTechniqueRequest = MoveTemp(Pending);
        OutReason = FString::Printf(
            TEXT("TechniqueRequestBuffered: from=%s to=%s elapsed=%.3f opens=%.3f lifetime=%.3f"),
            *ActivePlan.TechniqueId.ToString(),
            *TechniquePlan.Technique.TechniqueId.ToString(),
            CurrentState.Elapsed,
            Evaluation.Rule.MinElapsed,
            Lifetime);
        return ETechniqueBufferAttempt::Buffered;
    }

    if (Evaluation.Availability ==
        EKashmirTechniqueTransitionAvailability::WindowMissed)
    {
        OutReason = TEXT("TechniqueTransitionWindowMissed");
        return ETechniqueBufferAttempt::Rejected;
    }
    return ETechniqueBufferAttempt::NotApplicable;
}


void UKashmirDirectionalSwordComponent::UpdatePendingTechniqueRequest(
    const float DeltaSeconds)
{
    if (!PendingTechniqueRequest.IsSet())
    {
        return;
    }

    const FKashmirActionRuntimeState CurrentState = GetRuntimeState();
    if (!CurrentState.bActive)
    {
        ClearPendingTechniqueRequest(
            EKashmirPendingTechniqueClearReason::SourceEnded);
        return;
    }

    FKashmirPendingTechniqueRequestState& Pending =
        PendingTechniqueRequest.GetValue();
    if (Pending.SourceExecutionSerial != ActionExecutionSerial ||
        Pending.SourceActionId != CurrentState.ActionId ||
        Pending.SourceTechniqueId != ActivePlan.TechniqueId)
    {
        ClearPendingTechniqueRequest(
            EKashmirPendingTechniqueClearReason::SourceChanged);
        return;
    }

    Pending.Age += FMath::Max(0.0f, DeltaSeconds);
    if (Pending.Age - KINDA_SMALL_NUMBER > Pending.Lifetime)
    {
        ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::Expired);
        return;
    }

    const UKashmirWeaponCombatStyle* Style = Pending.Style.Get();
    FKashmirTechniqueActionPlan TechniquePlan;
    FKashmirSwordActionPlan Plan;
    FString Reason;
    if (!ResolveTechniqueRequestPlan(
            Pending.Request, Style, TechniquePlan, Plan, Reason))
    {
        ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::Invalid);
        return;
    }

    float SourceActionLifetime = 0.0f;
    if (!Runtime->GetActionTotalDuration(
            CurrentState.ActionId, SourceActionLifetime, Reason))
    {
        ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::Invalid);
        return;
    }
    const float RemainingLifetime =
        FMath::Max(0.0f, Pending.Lifetime - Pending.Age);
    const FKashmirTechniqueTransitionContext Context =
        BuildTechniqueTransitionContext(CurrentState, Pending.Request.ContextTags);
    FKashmirTechniqueTransitionEvaluation Evaluation;
    if (!Style->EvaluateTechniqueTransition(
            Pending.SourceTechniqueId,
            TechniquePlan.Technique.TechniqueId,
            Context,
            RemainingLifetime,
            SourceActionLifetime,
            Evaluation,
            Reason))
    {
        ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason::Invalid);
        return;
    }

    if (Evaluation.Availability ==
        EKashmirTechniqueTransitionAvailability::EligibleNow)
    {
        if (TryTransitionTechnique(
                Plan, Evaluation.Rule, Pending.Request.ContextTags, Reason))
        {
            ClearPendingTechniqueRequest(
                EKashmirPendingTechniqueClearReason::Consumed);
        }
        else
        {
            // v0.1 performs no resource prediction or hidden cancel fallback.
            // A failed preflight leaves the source intact and consumes intent.
            LastTechniqueRequestReason = Reason;
            ClearPendingTechniqueRequest(
                EKashmirPendingTechniqueClearReason::ResourceFailure);
        }
        return;
    }

    if (Evaluation.Availability ==
            EKashmirTechniqueTransitionAvailability::FutureWindowReachable ||
        Evaluation.Availability ==
            EKashmirTechniqueTransitionAvailability::OutcomePending)
    {
        return;
    }

    ClearPendingTechniqueRequest(
        Evaluation.Availability ==
            EKashmirTechniqueTransitionAvailability::WindowMissed
            ? EKashmirPendingTechniqueClearReason::WindowMissed
            : EKashmirPendingTechniqueClearReason::Invalid);
}


void UKashmirDirectionalSwordComponent::ClearPendingTechniqueRequest(
    const EKashmirPendingTechniqueClearReason Reason)
{
    if (!PendingTechniqueRequest.IsSet())
    {
        return;
    }
    PendingTechniqueRequest.Reset();
    LastPendingTechniqueClearReason = Reason;
}


bool UKashmirDirectionalSwordComponent::ValidateResolvedPlan(
    const FKashmirSwordActionPlan& Plan,
    FString& OutReason) const
{
    OutReason.Reset();
    if (!Plan.bResolved || !Plan.Gesture.ActionRequest.IsValid(OutReason) ||
        !Plan.RuntimeDefinition.IsValid(OutReason))
    {
        if (OutReason.IsEmpty())
        {
            OutReason = TEXT("sword action requires a resolved valid plan");
        }
        return false;
    }
    if (Plan.Montage.IsNull() ||
        !FMath::IsFinite(Plan.PlayRate) || Plan.PlayRate <= 0.0f)
    {
        OutReason = TEXT("sword action requires a montage and positive play rate");
        return false;
    }
    if (PresentationComponent != nullptr &&
        Plan.Montage.LoadSynchronous() == nullptr)
    {
        OutReason = TEXT("sword action presentation montage could not be loaded");
        return false;
    }
    if (Runtime == nullptr)
    {
        OutReason = TEXT("sword action requires an initialized runtime");
        return false;
    }
    if (Plan.MovementSpec.Delivery ==
        EKashmirMovementDelivery::ControlledTranslation)
    {
        float ActionLifetime = 0.0f;
        if (!Runtime->GetActionTotalDuration(
                Plan.Gesture.ActionRequest.ActionId,
                ActionLifetime,
                OutReason))
        {
            return false;
        }
        if (Plan.MovementSpec.Duration - KINDA_SMALL_NUMBER > ActionLifetime)
        {
            OutReason = FString::Printf(
                TEXT("MovementDurationExceedsActionLifetime: movement=%.3fs action=%.3fs action_id=%s"),
                Plan.MovementSpec.Duration,
                ActionLifetime,
                *Plan.Gesture.ActionRequest.ActionId.ToString());
            return false;
        }
    }
    if (MovementDeliveryComponent != nullptr)
    {
        return MovementDeliveryComponent->CanStartDelivery(
            Plan.MovementSpec, OutReason);
    }
    if (Plan.MovementSpec.Delivery != EKashmirMovementDelivery::None)
    {
        OutReason = TEXT("controlled translation requires a movement delivery component");
        return false;
    }
    return true;
}


bool UKashmirDirectionalSwordComponent::StartResolvedPlan(
    const FKashmirSwordActionPlan& Plan,
    FString& OutReason)
{
    if (Runtime == nullptr && !RebuildRuntime(OutReason))
    {
        return false;
    }
    if (!ValidateResolvedPlan(Plan, OutReason))
    {
        return false;
    }
    const FKashmirActionRuntimeState CurrentState = Runtime->GetState();
    const bool bWasActive = CurrentState.bActive;
    const bool bStarted = bWasActive
        ? Runtime->TryCancel(Plan.Gesture.ActionRequest, OutReason)
        : Runtime->Start(Plan.Gesture.ActionRequest, OutReason);
    if (!bStarted)
    {
        if (bWasActive)
        {
            const FString RuntimeReason = OutReason.IsEmpty()
                ? TEXT("runtime supplied no cancellation detail")
                : OutReason;
            OutReason = FString::Printf(
                TEXT("ActionCannotBeCancelled: current=%s phase=%d; %s"),
                *CurrentState.ActionId.ToString(),
                static_cast<int32>(CurrentState.Phase),
                *RuntimeReason);
        }
        return false;
    }

    ClearPendingTechniqueRequest(
        EKashmirPendingTechniqueClearReason::SourceChanged);
    if (bWasActive)
    {
        FinalizeCombatOutcome(EKashmirCombatOutcomeFinalizationReason::Interrupted);
    }
    ++ActionExecutionSerial;
    ActivePlan = Plan;
    const FKashmirActionRuntimeState StartedState = Runtime->GetState();
    BeginCombatOutcome(StartedState);
    if (MovementDeliveryComponent != nullptr &&
        !MovementDeliveryComponent->StartDelivery(
            Plan.MovementSpec, StartedState.ActionId, OutReason))
    {
        return false;
    }
    SynchronizeTraceWindow(CurrentState, StartedState);
    ApplyPresentation(StartedState);
    return true;
}


bool UKashmirDirectionalSwordComponent::TryTransitionTechnique(
    const FKashmirSwordActionPlan& Plan,
    const FKashmirTechniqueTransitionRule& TechniqueRule,
    const FGameplayTagContainer& ContextTags,
    FString& OutReason)
{
    if (Runtime == nullptr || !ActivePlan.bResolved)
    {
        OutReason = TEXT("technique transition requires an active resolved plan");
        return false;
    }
    if (!ValidateResolvedPlan(Plan, OutReason))
    {
        return false;
    }

    const FKashmirActionRuntimeState SourceState = Runtime->GetState();
    FKashmirTransitionRule ActionRule;
    ActionRule.FromActionId = SourceState.ActionId;
    ActionRule.ToActionId = Plan.Gesture.ActionRequest.ActionId;
    ActionRule.MinElapsed = TechniqueRule.MinElapsed;
    ActionRule.MaxElapsed = TechniqueRule.MaxElapsed;
    ActionRule.Priority = TechniqueRule.Priority;
    ActionRule.RequiredTags = TechniqueRule.RequiredTags;
    ActionRule.BlockedTags = TechniqueRule.BlockedTags;

    if (!Runtime->CanTransitionTo(
            Plan.Gesture.ActionRequest,
            ActionRule,
            ContextTags,
            OutReason))
    {
        return false;
    }

    // Commit the lower-level runtime first. Preflight above makes failure
    // non-destructive; no external A subsystem is ended until commit succeeds.
    if (!Runtime->TransitionTo(
            Plan.Gesture.ActionRequest,
            ActionRule,
            ContextTags,
            OutReason))
    {
        return false;
    }

    FinalizeCombatOutcome(EKashmirCombatOutcomeFinalizationReason::Transitioned);
    ++ActionExecutionSerial;
    if (WeaponTraceComponent != nullptr)
    {
        WeaponTraceComponent->EndTraceWindow();
    }
    if (MovementDeliveryComponent != nullptr)
    {
        MovementDeliveryComponent->TransitionDelivery();
    }
    if (PresentationComponent != nullptr)
    {
        PresentationComponent->StopPresentation();
    }

    ActivePlan = Plan;
    const FKashmirActionRuntimeState DestinationState = Runtime->GetState();
    BeginCombatOutcome(DestinationState);
    if (MovementDeliveryComponent != nullptr &&
        !MovementDeliveryComponent->StartDelivery(
            Plan.MovementSpec,
            DestinationState.ActionId,
            OutReason))
    {
        return false;
    }
    ResetTraceForTransition(DestinationState);
    ApplyPresentation(DestinationState);
    return true;
}


void UKashmirDirectionalSwordComponent::ResetTraceForTransition(
    const FKashmirActionRuntimeState& DestinationState)
{
    if (WeaponTraceComponent != nullptr &&
        DestinationState.bActive &&
        DestinationState.Phase == EKashmirActionPhase::Active)
    {
        WeaponTraceComponent->BeginTraceWindow();
    }
}


bool UKashmirDirectionalSwordComponent::CancelCurrentAction(FString& OutReason)
{
    OutReason.Reset();
    if (Runtime == nullptr || !ActivePlan.bResolved)
    {
        OutReason = TEXT("no active action to cancel");
        return false;
    }

    const FKashmirActionRuntimeState PreviousState = Runtime->GetState();
    if (!Runtime->TryCancelCurrent(OutReason))
    {
        const FString RuntimeReason = OutReason.IsEmpty()
            ? TEXT("runtime supplied no cancellation detail")
            : OutReason;
        OutReason = FString::Printf(
            TEXT("ActionCannotBeCancelled: current=%s phase=%d; %s"),
            *PreviousState.ActionId.ToString(),
            static_cast<int32>(PreviousState.Phase),
            *RuntimeReason);
        return false;
    }

    if (MovementDeliveryComponent != nullptr)
    {
        MovementDeliveryComponent->CancelDelivery();
    }
    FinalizeCombatOutcome(EKashmirCombatOutcomeFinalizationReason::Interrupted);
    ClearPendingTechniqueRequest(
        EKashmirPendingTechniqueClearReason::Cancelled);
    const FKashmirActionRuntimeState CancelledState = Runtime->GetState();
    SynchronizeTraceWindow(PreviousState, CancelledState);
    ApplyPresentation(CancelledState);
    ActivePlan = {};
    return true;
}


void UKashmirDirectionalSwordComponent::CancelGesture()
{
    bCapturingGesture = false;
    CapturedGesture = {};
    AccumulatedPosition = FVector2D::ZeroVector;
}


FKashmirActionRuntimeState
UKashmirDirectionalSwordComponent::GetRuntimeState() const
{
    return Runtime != nullptr
        ? Runtime->GetState()
        : FKashmirActionRuntimeState{};
}


TArray<FKashmirActionEvent>
UKashmirDirectionalSwordComponent::DrainRuntimeEvents()
{
    return Runtime != nullptr ? Runtime->DrainEvents() : TArray<FKashmirActionEvent>{};
}


void UKashmirDirectionalSwordComponent::ApplyPresentation(
    const FKashmirActionRuntimeState& RuntimeState)
{
    if (PresentationComponent == nullptr || !ActivePlan.bResolved)
    {
        return;
    }

    FString PresentationReason;
    if (!PresentationComponent->ApplyRuntimeState(
            ActivePlan,
            RuntimeState,
            PresentationReason))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Sword presentation failed for ActionId=%s: %s"),
            *RuntimeState.ActionId.ToString(),
            *PresentationReason);
    }
}


void UKashmirDirectionalSwordComponent::SynchronizeTraceWindow(
    const FKashmirActionRuntimeState& PreviousState,
    const FKashmirActionRuntimeState& CurrentState)
{
    if (WeaponTraceComponent == nullptr)
    {
        return;
    }

    const bool bWasActive = PreviousState.Phase == EKashmirActionPhase::Active;
    const bool bCurrentlyInActivePhase =
        CurrentState.Phase == EKashmirActionPhase::Active;
    if (!bWasActive && bCurrentlyInActivePhase)
    {
        WeaponTraceComponent->BeginTraceWindow();
    }
    else if (bWasActive && !bCurrentlyInActivePhase)
    {
        WeaponTraceComponent->EndTraceWindow();
    }

    if (!CurrentState.bActive && WeaponTraceComponent->IsTraceWindowActive())
    {
        WeaponTraceComponent->EndTraceWindow();
    }
}


bool UKashmirDirectionalSwordComponent::SampleWeaponTrace(
    float DeltaSeconds,
    FString& OutReason)
{
    OutReason.Reset();
    if (WeaponTraceComponent == nullptr ||
        !WeaponTraceComponent->IsTraceWindowActive())
    {
        return true;
    }

    TArray<FKashmirWeaponTraceHit> Hits;
    if (!WeaponTraceComponent->SampleTrace(DeltaSeconds, Hits, OutReason))
    {
        return false;
    }

    FKashmirDirectionalSwordContactResolver Resolver;
    for (const FKashmirWeaponTraceHit& Hit : Hits)
    {
        FKashmirDirectionalSwordContact Contact;
        if (!Resolver.Resolve(Hit, ActivePlan, Contact, OutReason))
        {
            return false;
        }
        OnSwordContact.Broadcast(Contact);
    }
    return true;
}


bool UKashmirDirectionalSwordComponent::RecordCombatOutcomeContact(
    const FKashmirCombatOutcomeContact& Contact,
    FString& OutReason)
{
    return CombatOutcome.RecordContact(Contact, OutReason);
}


void UKashmirDirectionalSwordComponent::BeginCombatOutcome(
    const FKashmirActionRuntimeState& RuntimeState)
{
    CombatOutcome.BeginExecution(
        static_cast<int64>(ActionExecutionSerial),
        ActivePlan.TechniqueId,
        RuntimeState.ActionId);
}


void UKashmirDirectionalSwordComponent::FinalizeCombatOutcome(
    const EKashmirCombatOutcomeFinalizationReason Reason)
{
    CombatOutcome.Finalize(Reason);
}


void UKashmirDirectionalSwordComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FString Reason;
    SampleWeaponTrace(DeltaTime, Reason);
    AdvanceRuntime(DeltaTime, Reason);
}


bool UKashmirDirectionalSwordComponent::AdvanceRuntime(
    float DeltaSeconds,
    FString& OutReason)
{
    OutReason.Reset();

    if (Runtime == nullptr || !ActivePlan.bResolved)
    {
        return true;
    }

    const FKashmirActionRuntimeState PreviousState = Runtime->GetState();
    if (MovementDeliveryComponent != nullptr &&
        MovementDeliveryComponent->IsDeliveryActive())
    {
        float ActionLifetime = PreviousState.Elapsed;
        if (!Runtime->GetActionTotalDuration(
                PreviousState.ActionId, ActionLifetime, OutReason))
        {
            return false;
        }
        const float ActionTimeRemaining =
            FMath::Max(0.0f, ActionLifetime - PreviousState.Elapsed);
        const float DeliveryDelta = FMath::Min(DeltaSeconds, ActionTimeRemaining);
        if (!MovementDeliveryComponent->AdvanceDelivery(
                DeliveryDelta, PreviousState, OutReason))
        {
            return false;
        }
    }
    if (!Runtime->Advance(DeltaSeconds, OutReason))
    {
        return false;
    }

    FKashmirActionRuntimeState RuntimeState = Runtime->GetState();
    UpdatePendingTechniqueRequest(DeltaSeconds);
    RuntimeState = Runtime->GetState();
    if (MovementDeliveryComponent != nullptr &&
        MovementDeliveryComponent->IsDeliveryActive() &&
        !RuntimeState.bActive &&
        !MovementDeliveryComponent->AdvanceDelivery(0.0f, RuntimeState, OutReason))
    {
        return false;
    }
    SynchronizeTraceWindow(PreviousState, RuntimeState);
    ApplyPresentation(RuntimeState);
    if (!RuntimeState.bActive)
    {
        FinalizeCombatOutcome(EKashmirCombatOutcomeFinalizationReason::Completed);
        ActivePlan = {};
    }
    return true;
}
