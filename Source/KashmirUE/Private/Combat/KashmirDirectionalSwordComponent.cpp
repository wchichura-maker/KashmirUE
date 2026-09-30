#include "Combat/KashmirDirectionalSwordComponent.h"

#include "Combat/KashmirSwordPresentationComponent.h"


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


void UKashmirDirectionalSwordComponent::SetProfile(
    UKashmirDirectionalSwordProfile* InProfile)
{
    if (WeaponTraceComponent != nullptr &&
        WeaponTraceComponent->IsTraceWindowActive())
    {
        WeaponTraceComponent->EndTraceWindow();
    }
    Profile = InProfile;
    CancelGesture();
    ActivePlan = {};
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


bool UKashmirDirectionalSwordComponent::RebuildRuntime(FString& OutReason)
{
    OutReason.Reset();
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


bool UKashmirDirectionalSwordComponent::StartTechniqueRequest(
    const FKashmirTechniqueRequest& Request,
    const UKashmirWeaponCombatStyle* Style,
    FString& OutReason)
{
    OutReason.Reset();
    LastTechniqueRequestReason.Reset();
    if (Style == nullptr)
    {
        OutReason = TEXT("technique request requires a weapon combat style");
        LastTechniqueRequestReason = OutReason;
        return false;
    }

    FKashmirTechniqueActionPlan TechniquePlan;
    if (!Style->ResolveTechnique(Request, TechniquePlan, OutReason))
    {
        LastTechniqueRequestReason = OutReason;
        return false;
    }

    // The existing profile remains the runtime/combat adapter during migration.
    // Technique data owns Base Motion and may override only presentation; it
    // never replaces trace, damage, hit evidence or ActionRuntime authority.
    FKashmirSwordActionPlan Plan;
    if (Profile == nullptr ||
        !Profile->ResolveActionPlan(
            TechniquePlan.ActionRequest,
            Plan,
            OutReason))
    {
        LastTechniqueRequestReason = OutReason;
        return false;
    }

    Plan.Montage = TechniquePlan.Technique.Montage;
    Plan.MontageSection = TechniquePlan.Technique.MontageSection;
    Plan.PlayRate = TechniquePlan.Technique.PlayRate;
    if (TechniquePlan.Technique.bOverrideSwordPresentation)
    {
        Plan.PoseConfig = TechniquePlan.Technique.SwordPresentation;
    }

    const bool bStarted = StartResolvedPlan(Plan, OutReason);
    if (!bStarted)
    {
        LastTechniqueRequestReason = OutReason;
    }
    return bStarted;
}


bool UKashmirDirectionalSwordComponent::StartResolvedPlan(
    const FKashmirSwordActionPlan& Plan,
    FString& OutReason)
{
    if (Runtime == nullptr && !RebuildRuntime(OutReason))
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

    ActivePlan = Plan;
    const FKashmirActionRuntimeState StartedState = Runtime->GetState();
    SynchronizeTraceWindow(CurrentState, StartedState);
    ApplyPresentation(StartedState);
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
    if (!Runtime->Advance(DeltaSeconds, OutReason))
    {
        return false;
    }

    const FKashmirActionRuntimeState RuntimeState = Runtime->GetState();
    SynchronizeTraceWindow(PreviousState, RuntimeState);
    ApplyPresentation(RuntimeState);
    if (!RuntimeState.bActive)
    {
        ActivePlan = {};
    }
    return true;
}
