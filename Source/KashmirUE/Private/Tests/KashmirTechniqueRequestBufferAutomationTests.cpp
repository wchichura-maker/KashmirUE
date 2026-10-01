#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


namespace KashmirTechniqueRequestBufferTests
{
    const FName TechniqueA(TEXT("Technique.Sword.Test.BufferA"));
    const FName TechniqueB(TEXT("Technique.Sword.Test.BufferB"));
    const FName TechniqueC(TEXT("Technique.Sword.Test.BufferC"));
    const FName ActionA(TEXT("Sword.Test.BufferA"));
    const FName ActionB(TEXT("Sword.Test.BufferB"));
    const FName ActionC(TEXT("Sword.Test.BufferC"));

    FKashmirSwordAuthoredAction MakeAction(
        const FName ActionId,
        const bool bCancellable = false)
    {
        FKashmirSwordAuthoredAction Result;
        Result.ActionId = ActionId;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_Buffer_Test.AM_Buffer_Test")));
        Result.StartupDuration = 0.18f;
        Result.ActiveDuration = 0.14f;
        Result.RecoveryDuration = 0.28f;
        Result.bCancellable = bCancellable;
        if (bCancellable)
        {
            Result.CancelWindows = {EKashmirActionPhase::Active};
        }
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        return Result;
    }

    FKashmirCombatTechniqueDefinition MakeTechnique(
        const FName TechniqueId,
        const FName ActionId)
    {
        FKashmirCombatTechniqueDefinition Result;
        Result.TechniqueId = TechniqueId;
        Result.ActionId = ActionId;
        Result.WeaponFamily = TEXT("Sword");
        Result.TechniqueFamily = TEXT("BufferTest");
        Result.AttackDirection = EKashmirAttackDirection::LeftToRight;
        Result.AttackShape = EKashmirAttackShape::Slash;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_Buffer_Test.AM_Buffer_Test")));
        Result.RuntimeDefinition = MakeAction(ActionId).BuildRuntimeDefinition();
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Result.BaseDamage = 10.0f;
        return Result;
    }

    FKashmirTechniqueTransitionRule MakeRule(
        const FName To = TechniqueB,
        const float Min = 0.32f,
        const float Max = 0.47f,
        const int32 Priority = 0)
    {
        FKashmirTechniqueTransitionRule Rule;
        Rule.FromTechniqueId = TechniqueA;
        Rule.ToTechniqueId = To;
        Rule.MinElapsed = Min;
        Rule.MaxElapsed = Max;
        Rule.Priority = Priority;
        return Rule;
    }

    FKashmirTechniqueRequest MakeRequest(
        const EKashmirTechniqueSlot Slot,
        const EKashmirTechniqueRequestSource Source =
            EKashmirTechniqueRequestSource::Player)
    {
        FKashmirTechniqueRequest Result;
        Result.Slot = Slot;
        Result.Source = Source;
        return Result;
    }

    struct FFixture
    {
        UKashmirDirectionalSwordProfile* Profile = nullptr;
        UKashmirWeaponCombatStyle* Style = nullptr;
        UKashmirDirectionalSwordComponent* Sword = nullptr;

        bool Initialize(
            const bool bSameActionId = false,
            const bool bCancellable = false,
            const bool bAddThirdTechnique = false)
        {
            Profile = NewObject<UKashmirDirectionalSwordProfile>();
            Profile->GestureConfig.MinimumDragDistance = 10.0f;
            Profile->GestureConfig.FullIntensityDistance = 100.0f;
            Profile->Actions.Add(MakeAction(ActionA, bCancellable));
            FKashmirSwordActionBinding ActionBindingA;
            ActionBindingA.Family = EKashmirSwordGestureFamily::Direct;
            ActionBindingA.Direction = EKashmirSwordGestureDirection::Right;
            ActionBindingA.ActionId = ActionA;
            Profile->GestureConfig.ActionBindings.Add(ActionBindingA);
            if (!bSameActionId)
            {
                Profile->Actions.Add(MakeAction(ActionB));
                FKashmirSwordActionBinding ActionBindingB;
                ActionBindingB.Family = EKashmirSwordGestureFamily::Direct;
                ActionBindingB.Direction = EKashmirSwordGestureDirection::Left;
                ActionBindingB.ActionId = ActionB;
                Profile->GestureConfig.ActionBindings.Add(ActionBindingB);
            }
            if (bAddThirdTechnique)
            {
                Profile->Actions.Add(MakeAction(ActionC));
                FKashmirSwordActionBinding ActionBindingC;
                ActionBindingC.Family = EKashmirSwordGestureFamily::Direct;
                ActionBindingC.Direction = EKashmirSwordGestureDirection::Up;
                ActionBindingC.ActionId = ActionC;
                Profile->GestureConfig.ActionBindings.Add(ActionBindingC);
            }

            Style = NewObject<UKashmirWeaponCombatStyle>();
            Style->StyleId = TEXT("Style.Sword.BufferTest");
            Style->WeaponFamily = TEXT("Sword");
            Style->Techniques.Add(MakeTechnique(TechniqueA, ActionA));
            Style->Techniques.Add(MakeTechnique(
                TechniqueB, bSameActionId ? ActionA : ActionB));
            if (bAddThirdTechnique)
            {
                Style->Techniques.Add(MakeTechnique(TechniqueC, ActionC));
            }

            FKashmirTechniqueSlotBinding BindingA;
            BindingA.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
            BindingA.TechniqueId = TechniqueA;
            Style->SlotBindings.Add(BindingA);
            FKashmirTechniqueSlotBinding BindingB;
            BindingB.Slot = EKashmirTechniqueSlot::TechniqueSlot2;
            BindingB.TechniqueId = TechniqueB;
            Style->SlotBindings.Add(BindingB);
            if (bAddThirdTechnique)
            {
                FKashmirTechniqueSlotBinding BindingC;
                BindingC.Slot = EKashmirTechniqueSlot::TechniqueSlot3;
                BindingC.TechniqueId = TechniqueC;
                Style->SlotBindings.Add(BindingC);
            }

            Sword = NewObject<UKashmirDirectionalSwordComponent>();
            Sword->SetProfile(Profile);
            return true;
        }

        bool StartSource(FString& OutReason)
        {
            const bool bStarted = Sword->StartTechniqueRequest(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), Style, OutReason);
            Sword->DrainRuntimeEvents();
            return bStarted;
        }

        bool Advance(const float Seconds, FString& OutReason)
        {
            return Sword->AdvanceRuntime(Seconds, OutReason);
        }
    };

    bool ContainsEvent(
        const TArray<FKashmirActionEvent>& Events,
        const EKashmirActionEventType Type)
    {
        return Events.ContainsByPredicate(
            [Type](const FKashmirActionEvent& Event)
            {
                return Event.Type == Type;
            });
    }
}


#define KASHMIR_BUFFER_TEST(TypeName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
        TypeName, \
        "Kashmir.Combat.TechniqueRequestBuffer." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


namespace KashmirTechniqueRequestBufferTests
{


KASHMIR_BUFFER_TEST(FBufferAdmissionTest, "Admission")
bool FBufferAdmissionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture NoActive;
    NoActive.Initialize();
    NoActive.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("No active Action executes normally instead of buffering"),
        NoActive.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2),
            NoActive.Style,
            Reason));
    TestFalse(TEXT("No active Action creates no pending request"),
        NoActive.Sword->HasPendingTechniqueRequest());

    FFixture Unbound;
    Unbound.Initialize();
    Unbound.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Unbound fixture source starts"), Unbound.StartSource(Reason));
    TestFalse(TEXT("Slot5 remains safe rejected"),
        Unbound.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot5),
            Unbound.Style,
            Reason));
    TestFalse(TEXT("Slot5 never enters the buffer"),
        Unbound.Sword->HasPendingTechniqueRequest());

    FFixture EmptyRules;
    EmptyRules.Initialize();
    TestTrue(TEXT("Empty-rule source starts"), EmptyRules.StartSource(Reason));
    TestTrue(TEXT("Empty-rule source advances"), EmptyRules.Advance(0.22f, Reason));
    TestFalse(TEXT("No transition rule preserves prior rejection behavior"),
        EmptyRules.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2),
            EmptyRules.Style,
            Reason));
    TestFalse(TEXT("Empty rules create no pending request"),
        EmptyRules.Sword->HasPendingTechniqueRequest());
    return true;
}


KASHMIR_BUFFER_TEST(FBufferBoundaryTest, "TemporalBoundaries")
bool FBufferBoundaryTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Before;
    Before.Initialize();
    Before.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Before source starts"), Before.StartSource(Reason));
    TestTrue(TEXT("Before source reaches just before Min"),
        Before.Advance(0.319f, Reason));
    TestTrue(TEXT("Just-before-Min request is accepted into buffer"),
        Before.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Before.Style, Reason));
    TestTrue(TEXT("Just-before-Min request is pending"),
        Before.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Source remains active while request is pending"),
        Before.Sword->GetActivePlan().TechniqueId, TechniqueA);

    for (const float Elapsed : {0.32f, 0.35f, 0.47f})
    {
        FFixture Immediate;
        Immediate.Initialize();
        Immediate.Style->TransitionRules = {MakeRule()};
        TestTrue(TEXT("Immediate source starts"), Immediate.StartSource(Reason));
        TestTrue(TEXT("Immediate source reaches requested boundary"),
            Immediate.Advance(Elapsed, Reason));
        TestTrue(*FString::Printf(TEXT("Request at %.3f transitions"), Elapsed),
            Immediate.Sword->StartTechniqueRequest(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2),
                Immediate.Style,
                Reason));
        TestEqual(TEXT("Immediate boundary activates destination Technique"),
            Immediate.Sword->GetActivePlan().TechniqueId, TechniqueB);
        TestFalse(TEXT("Immediate transition creates no pending request"),
            Immediate.Sword->HasPendingTechniqueRequest());
    }

    FFixture After;
    After.Initialize();
    After.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("After source starts"), After.StartSource(Reason));
    TestTrue(TEXT("After source advances beyond Max"), After.Advance(0.471f, Reason));
    TestFalse(TEXT("After-window request is rejected"),
        After.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), After.Style, Reason));
    TestFalse(TEXT("After-window request is not buffered"),
        After.Sword->HasPendingTechniqueRequest());
    return true;
}


KASHMIR_BUFFER_TEST(FBufferConsumptionTest, "Consumption")
bool FBufferConsumptionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    Fixture.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Source starts"), Fixture.StartSource(Reason));
    TestTrue(TEXT("Source advances to early-input time"), Fixture.Advance(0.22f, Reason));
    const FKashmirTechniqueRequest ReplayRequest = MakeRequest(
        EKashmirTechniqueSlot::TechniqueSlot2,
        EKashmirTechniqueRequestSource::Replay);
    TestTrue(TEXT("Early request is accepted"),
        Fixture.Sword->StartTechniqueRequest(ReplayRequest, Fixture.Style, Reason));
    TestTrue(TEXT("Early request remains pending"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Pending preserves Technique identity"),
        Fixture.Sword->GetPendingTechniqueId(), TechniqueB);
    TestEqual(TEXT("Pending preserves request Source"),
        Fixture.Sword->GetPendingTechniqueSource(),
        EKashmirTechniqueRequestSource::Replay);
    TestEqual(TEXT("Buffer capture uses authoritative Action elapsed"),
        Fixture.Sword->GetPendingTechniqueBufferedAtActionElapsed(), 0.22f);
    TestEqual(TEXT("Buffer lifetime is the documented 0.20 s"),
        Fixture.Sword->GetPendingTechniqueLifetime(), 0.20f);
    TestEqual(TEXT("Destination has not started early"),
        Fixture.Sword->GetActivePlan().TechniqueId, TechniqueA);

    TestTrue(TEXT("Runtime advances to transition opening"),
        Fixture.Advance(0.10f, Reason));
    TestFalse(TEXT("Successful consumption clears pending"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Pending clear reason is Consumed"),
        Fixture.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::Consumed);
    TestEqual(TEXT("Destination Technique becomes active"),
        Fixture.Sword->GetActivePlan().TechniqueId, TechniqueB);
    const TArray<FKashmirActionEvent> Events = Fixture.Sword->DrainRuntimeEvents();
    TestEqual(TEXT("Consumption emits exactly two runtime events"), Events.Num(), 2);
    TestEqual(TEXT("Consumption emits Transitioned first"),
        Events[0].Type, EKashmirActionEventType::Transitioned);
    TestEqual(TEXT("Consumption emits Started second"),
        Events[1].Type, EKashmirActionEventType::Started);
    TestFalse(TEXT("Consumption never emits Interrupted"),
        ContainsEvent(Events, EKashmirActionEventType::Interrupted));
    return true;
}


KASHMIR_BUFFER_TEST(FBufferIdentityTest, "IdentityAndReplacement")
bool FBufferIdentityTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Latest;
    Latest.Initialize(false, false, true);
    Latest.Style->TransitionRules = {MakeRule(TechniqueB), MakeRule(TechniqueC)};
    TestTrue(TEXT("Latest source starts"), Latest.StartSource(Reason));
    TestTrue(TEXT("Latest source advances"), Latest.Advance(0.22f, Reason));
    TestTrue(TEXT("First valid request buffers"),
        Latest.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Latest.Style, Reason));
    TestTrue(TEXT("Second valid request replaces first"),
        Latest.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot3), Latest.Style, Reason));
    TestEqual(TEXT("Latest valid request wins"),
        Latest.Sword->GetPendingTechniqueId(), TechniqueC);
    TestEqual(TEXT("Supersession is observable"),
        Latest.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::Superseded);

    FFixture Refresh;
    Refresh.Initialize();
    Refresh.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Refresh source starts"), Refresh.StartSource(Reason));
    TestTrue(TEXT("Refresh source advances"), Refresh.Advance(0.22f, Reason));
    TestTrue(TEXT("Refresh request buffers"), Refresh.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Refresh.Style, Reason));
    TestTrue(TEXT("Pending age advances"), Refresh.Advance(0.05f, Reason));
    TestTrue(TEXT("Pending age is non-zero"),
        Refresh.Sword->GetPendingTechniqueAge() > 0.0f);
    TestTrue(TEXT("Repeated same request refreshes pending"),
        Refresh.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Refresh.Style, Reason));
    TestEqual(TEXT("Repeated request resets age"),
        Refresh.Sword->GetPendingTechniqueAge(), 0.0f);
    TestEqual(TEXT("Repeated request retains a single identity"),
        Refresh.Sword->GetPendingTechniqueId(), TechniqueB);

    FFixture SameAction;
    SameAction.Initialize(true);
    SameAction.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Shared-Action source starts"), SameAction.StartSource(Reason));
    TestTrue(TEXT("Shared-Action source advances"), SameAction.Advance(0.22f, Reason));
    TestTrue(TEXT("Different Technique sharing ActionId buffers"),
        SameAction.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2),
            SameAction.Style,
            Reason));
    TestEqual(TEXT("Pending identity is Technique, not Action"),
        SameAction.Sword->GetPendingTechniqueId(), TechniqueB);
    TestTrue(TEXT("Shared-Action pending consumes"),
        SameAction.Advance(0.10f, Reason));
    TestEqual(TEXT("Shared ActionId still changes Technique identity"),
        SameAction.Sword->GetActivePlan().TechniqueId, TechniqueB);
    TestEqual(TEXT("Shared ActionId remains stable"),
        SameAction.Sword->GetRuntimeState().ActionId, ActionA);
    return true;
}


KASHMIR_BUFFER_TEST(FBufferClearingTest, "ClearConditions")
bool FBufferClearingTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Expired;
    Expired.Initialize();
    Expired.Style->TransitionRules = {MakeRule(TechniqueB, 0.40f, 0.50f)};
    TestTrue(TEXT("Expiry source starts"), Expired.StartSource(Reason));
    TestTrue(TEXT("Expiry source advances"), Expired.Advance(0.22f, Reason));
    TestTrue(TEXT("Expiry request buffers"), Expired.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Expired.Style, Reason));
    TestTrue(TEXT("Expiry update remains operational"), Expired.Advance(0.21f, Reason));
    TestFalse(TEXT("Expired request is removed"),
        Expired.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Expiration reason is deterministic"),
        Expired.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::Expired);
    TestEqual(TEXT("Expired destination never starts"),
        Expired.Sword->GetActivePlan().TechniqueId, TechniqueA);
    TestFalse(TEXT("Expiration emits no delayed transition"),
        ContainsEvent(Expired.Sword->DrainRuntimeEvents(),
            EKashmirActionEventType::Transitioned));

    FFixture Completed;
    Completed.Initialize();
    Completed.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Completion source starts"), Completed.StartSource(Reason));
    TestTrue(TEXT("Completion source advances"), Completed.Advance(0.22f, Reason));
    TestTrue(TEXT("Completion request buffers"), Completed.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Completed.Style, Reason));
    TestTrue(TEXT("Source completes normally"), Completed.Advance(0.50f, Reason));
    TestFalse(TEXT("Completion clears pending"),
        Completed.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Completion clear reason is SourceEnded"),
        Completed.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::SourceEnded);

    FFixture Cancelled;
    Cancelled.Initialize(false, true);
    Cancelled.Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Cancellation source starts"), Cancelled.StartSource(Reason));
    TestTrue(TEXT("Cancellation source enters Active"), Cancelled.Advance(0.22f, Reason));
    TestTrue(TEXT("Cancellation request buffers"), Cancelled.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Cancelled.Style, Reason));
    TestTrue(TEXT("Explicit cancellation succeeds"),
        Cancelled.Sword->CancelCurrentAction(Reason));
    TestFalse(TEXT("Cancellation clears pending"),
        Cancelled.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Cancellation reason is observable"),
        Cancelled.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::Cancelled);

    FFixture Transitioned;
    Transitioned.Initialize(false, false, true);
    Transitioned.Style->TransitionRules = {
        MakeRule(TechniqueB), MakeRule(TechniqueC, 0.20f, 0.30f)};
    TestTrue(TEXT("Transition source starts"), Transitioned.StartSource(Reason));
    TestTrue(TEXT("Transition source enters immediate C window"),
        Transitioned.Advance(0.22f, Reason));
    TestTrue(TEXT("Old B intent buffers"), Transitioned.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Transitioned.Style, Reason));
    TestTrue(TEXT("Immediate C transition succeeds"),
        Transitioned.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot3),
            Transitioned.Style,
            Reason));
    TestFalse(TEXT("Source transition clears old pending"),
        Transitioned.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Old pending clears as SourceChanged"),
        Transitioned.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::SourceChanged);
    return true;
}


KASHMIR_BUFFER_TEST(FBufferResourceFailureTest, "ResourceFailure")
bool FBufferResourceFailureTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    Fixture.Style->TransitionRules = {MakeRule()};
    const FGameplayTag Stamina =
        FGameplayTag::RequestGameplayTag(TEXT("Resource.Stamina"));
    FKashmirResourceCost Cost;
    Cost.ResourceTag = Stamina;
    Cost.Amount = 10.0f;
    Fixture.Profile->Actions[1].StartCosts = {Cost};
    Fixture.Sword->SetProfile(Fixture.Profile);
    TestTrue(TEXT("Resource source starts"), Fixture.StartSource(Reason));
    TestTrue(TEXT("Resource source advances"), Fixture.Advance(0.22f, Reason));
    TestTrue(TEXT("Resource request buffers before preflight"),
        Fixture.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Fixture.Style, Reason));
    TestTrue(TEXT("Resource update remains operational"), Fixture.Advance(0.10f, Reason));
    TestFalse(TEXT("Unpayable pending is discarded"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Resource failure reason is deterministic"),
        Fixture.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::ResourceFailure);
    TestEqual(TEXT("Resource failure leaves source Technique intact"),
        Fixture.Sword->GetActivePlan().TechniqueId, TechniqueA);
    TestEqual(TEXT("Resource failure leaves source Action intact"),
        Fixture.Sword->GetRuntimeState().ActionId, ActionA);
    const TArray<FKashmirActionEvent> Events = Fixture.Sword->DrainRuntimeEvents();
    TestFalse(TEXT("Resource failure emits no transition"),
        ContainsEvent(Events, EKashmirActionEventType::Transitioned));
    TestFalse(TEXT("Resource failure emits no interruption"),
        ContainsEvent(Events, EKashmirActionEventType::Interrupted));
    return true;
}


KASHMIR_BUFFER_TEST(FBufferAuthorityTest, "AuthorityBoundaries")
bool FBufferAuthorityTest::RunTest(const FString& Parameters)
{
    FString RuntimeSource;
    FString SwordSource;
    const FString RuntimePath = FPaths::Combine(
        FPaths::ProjectDir(),
        TEXT("Source/KashmirUE/Private/Runtime/KashmirActionRuntime.cpp"));
    const FString SwordPath = FPaths::Combine(
        FPaths::ProjectDir(),
        TEXT("Source/KashmirUE/Private/Combat/KashmirDirectionalSwordComponent.cpp"));
    TestTrue(TEXT("ActionRuntime source is readable"),
        FFileHelper::LoadFileToString(RuntimeSource, *RuntimePath));
    TestTrue(TEXT("DirectionalSword source is readable"),
        FFileHelper::LoadFileToString(SwordSource, *SwordPath));
    TestFalse(TEXT("ActionRuntime remains Technique-buffer agnostic"),
        RuntimeSource.Contains(TEXT("PendingTechnique")) ||
        RuntimeSource.Contains(TEXT("TechniqueRequestBuffer")) ||
        RuntimeSource.Contains(TEXT("TechniqueId")));
    TestTrue(TEXT("DirectionalSword owns pending request lifecycle"),
        SwordSource.Contains(TEXT("UpdatePendingTechniqueRequest")));
    TestFalse(TEXT("Technique buffer introduces no Root Motion authority"),
        SwordSource.Contains(TEXT("RootMotion")));
    return true;
}


}


#undef KASHMIR_BUFFER_TEST

#endif
