#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Misc/AutomationTest.h"


namespace KashmirOutcomeConditionedTransitionTests
{
    const FName TechniqueA(TEXT("Technique.Sword.Test.OutcomeCondition.A"));
    const FName TechniqueB(TEXT("Technique.Sword.Test.OutcomeCondition.B"));
    const FName TechniqueC(TEXT("Technique.Sword.Test.OutcomeCondition.C"));
    const FName ActionA(TEXT("Sword.Test.OutcomeCondition.A"));
    const FName ActionB(TEXT("Sword.Test.OutcomeCondition.B"));
    const FName ActionC(TEXT("Sword.Test.OutcomeCondition.C"));

    int32 FactMask(const EKashmirCombatOutcomeFact Fact)
    {
        return static_cast<int32>(Fact);
    }

    FKashmirSwordAuthoredAction MakeAction(const FName ActionId)
    {
        FKashmirSwordAuthoredAction Result;
        Result.ActionId = ActionId;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_OutcomeCondition_Test.AM_OutcomeCondition_Test")));
        Result.StartupDuration = 0.18f;
        Result.ActiveDuration = 0.14f;
        Result.RecoveryDuration = 0.28f;
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
        Result.TechniqueFamily = TEXT("OutcomeConditionTest");
        Result.AttackDirection = EKashmirAttackDirection::LeftToRight;
        Result.AttackShape = EKashmirAttackShape::Slash;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_OutcomeCondition_Test.AM_OutcomeCondition_Test")));
        Result.RuntimeDefinition = MakeAction(ActionId).BuildRuntimeDefinition();
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Result.BaseDamage = 10.0f;
        return Result;
    }

    FKashmirTechniqueTransitionRule MakeRule(
        const FName To = TechniqueB,
        const int32 RequiredFacts = 0,
        const float MinElapsed = 0.32f,
        const float MaxElapsed = 0.47f,
        const int32 Priority = 0)
    {
        FKashmirTechniqueTransitionRule Rule;
        Rule.FromTechniqueId = TechniqueA;
        Rule.ToTechniqueId = To;
        Rule.MinElapsed = MinElapsed;
        Rule.MaxElapsed = MaxElapsed;
        Rule.Priority = Priority;
        Rule.RequiredOutcomeFacts = RequiredFacts;
        return Rule;
    }

    FKashmirTechniqueRequest MakeRequest(const EKashmirTechniqueSlot Slot)
    {
        FKashmirTechniqueRequest Result;
        Result.Slot = Slot;
        Result.Source = EKashmirTechniqueRequestSource::Player;
        return Result;
    }

    UKashmirWeaponCombatStyle* MakeStyle(const bool bSameActionId = false)
    {
        UKashmirWeaponCombatStyle* Style = NewObject<UKashmirWeaponCombatStyle>();
        Style->StyleId = TEXT("Style.Sword.OutcomeConditionTest");
        Style->WeaponFamily = TEXT("Sword");
        Style->Techniques = {
            MakeTechnique(TechniqueA, ActionA),
            MakeTechnique(TechniqueB, bSameActionId ? ActionA : ActionB),
            MakeTechnique(TechniqueC, ActionC)};
        FKashmirTechniqueSlotBinding A;
        A.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
        A.TechniqueId = TechniqueA;
        FKashmirTechniqueSlotBinding B;
        B.Slot = EKashmirTechniqueSlot::TechniqueSlot2;
        B.TechniqueId = TechniqueB;
        FKashmirTechniqueSlotBinding C;
        C.Slot = EKashmirTechniqueSlot::TechniqueSlot3;
        C.TechniqueId = TechniqueC;
        Style->SlotBindings = {A, B, C};
        return Style;
    }

    struct FFixture
    {
        UKashmirDirectionalSwordProfile* Profile = nullptr;
        UKashmirWeaponCombatStyle* Style = nullptr;
        UKashmirDirectionalSwordComponent* Sword = nullptr;

        void Initialize(const bool bSameActionId = false)
        {
            Profile = NewObject<UKashmirDirectionalSwordProfile>();
            Profile->GestureConfig.MinimumDragDistance = 10.0f;
            Profile->GestureConfig.FullIntensityDistance = 100.0f;
            Profile->Actions.Add(MakeAction(ActionA));
            if (!bSameActionId)
            {
                Profile->Actions.Add(MakeAction(ActionB));
            }
            Profile->Actions.Add(MakeAction(ActionC));

            FKashmirSwordActionBinding A;
            A.Family = EKashmirSwordGestureFamily::Direct;
            A.Direction = EKashmirSwordGestureDirection::Right;
            A.ActionId = ActionA;
            FKashmirSwordActionBinding B;
            B.Family = EKashmirSwordGestureFamily::Direct;
            B.Direction = EKashmirSwordGestureDirection::Left;
            B.ActionId = bSameActionId ? ActionA : ActionB;
            FKashmirSwordActionBinding C;
            C.Family = EKashmirSwordGestureFamily::Direct;
            C.Direction = EKashmirSwordGestureDirection::Up;
            C.ActionId = ActionC;
            Profile->GestureConfig.ActionBindings = {A, B, C};

            Style = MakeStyle(bSameActionId);
            Sword = NewObject<UKashmirDirectionalSwordComponent>();
            Sword->SetProfile(Profile);
        }

        bool StartA(FString& Reason)
        {
            const bool bStarted = Sword->StartTechniqueRequest(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), Style, Reason);
            Sword->DrainRuntimeEvents();
            return bStarted;
        }

        bool RequestB(FString& Reason)
        {
            return Sword->StartTechniqueRequest(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Style, Reason);
        }

        bool Record(
            const bool bDamage,
            const bool bBlocked,
            const bool bParried,
            const bool bGuardBroken,
            FString& Reason)
        {
            FKashmirCombatOutcomeContact Contact;
            Contact.TargetId = TEXT("Target.OutcomeCondition");
            Contact.bDamageApplied = bDamage;
            Contact.DamageApplied = bDamage ? 10.0f : 0.0f;
            Contact.bBlocked = bBlocked;
            Contact.bParried = bParried;
            Contact.bGuardBroken = bGuardBroken;
            Contact.CombatResult.bDelivered = true;
            return Sword->RecordCombatOutcomeContact(Contact, Reason);
        }
    };
}


#define KASHMIR_OUTCOME_CONDITION_TEST(TypeName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
        TypeName, \
        "Kashmir.Combat.OutcomeConditionedTransitions." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


namespace KashmirOutcomeConditionedTransitionTests
{


KASHMIR_OUTCOME_CONDITION_TEST(FOutcomeFactContractTest, "FactContract")
bool FOutcomeFactContractTest::RunTest(const FString& Parameters)
{
    FKashmirCombatExecutionOutcome Outcome;
    Outcome.bHadContact = true;
    Outcome.bAppliedDamage = true;
    Outcome.bWasBlocked = true;
    Outcome.bWasParried = true;
    Outcome.bCausedGuardBreak = true;
    const FKashmirCombatOutcomeFacts Facts =
        FKashmirCombatOutcomeFacts::FromOutcome(Outcome);
    TestTrue(TEXT("Zero requirements preserve legacy behavior"), Facts.Satisfies(0));
    TestTrue(TEXT("HadContact projects"),
        Facts.Satisfies(FactMask(EKashmirCombatOutcomeFact::HadContact)));
    TestTrue(TEXT("AppliedDamage projects"),
        Facts.Satisfies(FactMask(EKashmirCombatOutcomeFact::AppliedDamage)));
    TestTrue(TEXT("Blocked projects"),
        Facts.Satisfies(FactMask(EKashmirCombatOutcomeFact::Blocked)));
    TestTrue(TEXT("Parried projects"),
        Facts.Satisfies(FactMask(EKashmirCombatOutcomeFact::Parried)));
    TestTrue(TEXT("GuardBroken projects"),
        Facts.Satisfies(FactMask(EKashmirCombatOutcomeFact::GuardBroken)));
    const int32 AndMask = FactMask(EKashmirCombatOutcomeFact::HadContact) |
        FactMask(EKashmirCombatOutcomeFact::AppliedDamage);
    TestTrue(TEXT("All configured facts use AND semantics"), Facts.Satisfies(AndMask));
    FKashmirCombatOutcomeFacts Partial;
    Partial.bHadContact = true;
    TestFalse(TEXT("Partially satisfied AND is rejected"), Partial.Satisfies(AndMask));
    TestFalse(TEXT("Unknown bits are rejected"),
        FKashmirCombatOutcomeFacts::IsValidRequirementMask(1 << 7));
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FOutcomeEligibilityTest, "EligibilityMatrix")
bool FOutcomeEligibilityTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    FString Reason;
    FKashmirTechniqueTransitionRule Resolved;
    FKashmirTechniqueTransitionContext Context;
    Context.Elapsed = 0.35f;

    struct FCase
    {
        EKashmirCombatOutcomeFact Fact;
        bool FKashmirCombatOutcomeFacts::* Member;
    };
    const FCase Cases[] = {
        {EKashmirCombatOutcomeFact::HadContact, &FKashmirCombatOutcomeFacts::bHadContact},
        {EKashmirCombatOutcomeFact::AppliedDamage, &FKashmirCombatOutcomeFacts::bAppliedDamage},
        {EKashmirCombatOutcomeFact::Blocked, &FKashmirCombatOutcomeFacts::bBlocked},
        {EKashmirCombatOutcomeFact::Parried, &FKashmirCombatOutcomeFacts::bParried},
        {EKashmirCombatOutcomeFact::GuardBroken, &FKashmirCombatOutcomeFacts::bGuardBroken}};
    for (const FCase& Case : Cases)
    {
        Style->TransitionRules = {MakeRule(TechniqueB, FactMask(Case.Fact))};
        Context.OutcomeFacts = {};
        TestFalse(TEXT("Missing positive fact is not currently eligible"),
            Style->ResolveTechniqueTransition(
                TechniqueA, TechniqueB, Context, Resolved, Reason));
        Context.OutcomeFacts.*Case.Member = true;
        TestTrue(TEXT("Present positive fact is eligible"),
            Style->ResolveTechniqueTransition(
                TechniqueA, TechniqueB, Context, Resolved, Reason));
    }

    Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::AppliedDamage))};
    Context.OutcomeFacts = {};
    Context.OutcomeFacts.bHadContact = true;
    TestFalse(TEXT("Contact without damage does not satisfy AppliedDamage"),
        Style->ResolveTechniqueTransition(
            TechniqueA, TechniqueB, Context, Resolved, Reason));
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FAvailabilityTest, "Availability")
bool FAvailabilityTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::AppliedDamage))};
    FKashmirTechniqueTransitionContext Context;
    FKashmirTechniqueTransitionEvaluation Evaluation;
    FString Reason;

    Context.Elapsed = 0.22f;
    TestTrue(TEXT("Future evaluation succeeds"), Style->EvaluateTechniqueTransition(
        TechniqueA, TechniqueB, Context, 0.20f, 0.60f, Evaluation, Reason));
    TestEqual(TEXT("Before window is future reachable"), Evaluation.Availability,
        EKashmirTechniqueTransitionAvailability::FutureWindowReachable);
    Context.Elapsed = 0.35f;
    Style->EvaluateTechniqueTransition(
        TechniqueA, TechniqueB, Context, 0.20f, 0.60f, Evaluation, Reason);
    TestEqual(TEXT("Open window without evidence is OutcomePending"),
        Evaluation.Availability,
        EKashmirTechniqueTransitionAvailability::OutcomePending);
    Context.OutcomeFacts.bAppliedDamage = true;
    Style->EvaluateTechniqueTransition(
        TechniqueA, TechniqueB, Context, 0.20f, 0.60f, Evaluation, Reason);
    TestEqual(TEXT("Open window with evidence is EligibleNow"),
        Evaluation.Availability,
        EKashmirTechniqueTransitionAvailability::EligibleNow);
    Context.Elapsed = 0.50f;
    Style->EvaluateTechniqueTransition(
        TechniqueA, TechniqueB, Context, 0.20f, 0.60f, Evaluation, Reason);
    TestEqual(TEXT("Closed window is WindowMissed"), Evaluation.Availability,
        EKashmirTechniqueTransitionAvailability::WindowMissed);
    Style->EvaluateTechniqueTransition(
        TechniqueC, TechniqueB, Context, 0.20f, 0.60f, Evaluation, Reason);
    TestEqual(TEXT("Unknown edge has no matching rule"), Evaluation.Availability,
        EKashmirTechniqueTransitionAvailability::NoMatchingRule);
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FPriorityTest, "PriorityAndOrdering")
bool FPriorityTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    Style->TransitionRules = {
        MakeRule(TechniqueB, FactMask(EKashmirCombatOutcomeFact::AppliedDamage),
            0.0f, -1.0f, 20),
        MakeRule(TechniqueC, 0, 0.0f, -1.0f, 10)};
    FKashmirTechniqueTransitionContext Context;
    Context.Elapsed = 0.10f;
    const TArray<FName> WithoutDamage =
        Style->GetTechniqueTransitionOptions(TechniqueA, Context);
    TestEqual(TEXT("Unsatisfied high priority does not hide fallback"),
        WithoutDamage.Num(), 1);
    TestEqual(TEXT("Satisfied lower priority remains available"),
        WithoutDamage[0], TechniqueC);
    Context.OutcomeFacts.bAppliedDamage = true;
    const TArray<FName> WithDamage =
        Style->GetTechniqueTransitionOptions(TechniqueA, Context);
    TestEqual(TEXT("Both satisfied destinations are returned"), WithDamage.Num(), 2);
    TestEqual(TEXT("Higher priority sorts first"), WithDamage[0], TechniqueB);

    Style->TransitionRules = {
        MakeRule(TechniqueC, 0, 0.0f, -1.0f, 10),
        MakeRule(TechniqueB, 0, 0.0f, -1.0f, 10)};
    const TArray<FName> Lexical =
        Style->GetTechniqueTransitionOptions(TechniqueA, Context);
    TestTrue(TEXT("Equal priority uses lexical destination order"),
        Lexical[0].LexicalLess(Lexical[1]));
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FValidationTest, "ValidationAndDuplicates")
bool FValidationTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    FString Reason;
    const int32 Damage = FactMask(EKashmirCombatOutcomeFact::AppliedDamage);
    const int32 Blocked = FactMask(EKashmirCombatOutcomeFact::Blocked);
    Style->TransitionRules = {MakeRule(TechniqueB, Damage), MakeRule(TechniqueB, Damage)};
    TestFalse(TEXT("Identical outcome masks are semantic duplicates"),
        Style->ValidateStyle(Reason));
    Style->TransitionRules = {MakeRule(TechniqueB, Damage), MakeRule(TechniqueB, Blocked)};
    TestTrue(TEXT("Different outcome masks are distinct rules"),
        Style->ValidateStyle(Reason));
    Style->TransitionRules = {MakeRule(TechniqueB, 1 << 7)};
    TestFalse(TEXT("Invalid outcome bits are rejected"), Style->ValidateStyle(Reason));
    Style->TransitionRules = {MakeRule()};
    TestTrue(TEXT("Zero requirements remain valid"), Style->ValidateStyle(Reason));
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FImmediateTransitionTest, "ImmediateAndSameAction")
bool FImmediateTransitionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    Fixture.Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::HadContact))};
    TestTrue(TEXT("A starts"), Fixture.StartA(Reason));
    TestTrue(TEXT("Contact records"), Fixture.Record(false, false, false, false, Reason));
    TestTrue(TEXT("A reaches window"), Fixture.Sword->AdvanceRuntime(0.35f, Reason));
    TestTrue(TEXT("Conditioned transition is immediate"), Fixture.RequestB(Reason));
    TestEqual(TEXT("B is active"), Fixture.Sword->GetActivePlan().TechniqueId, TechniqueB);

    FFixture SameAction;
    SameAction.Initialize(true);
    SameAction.Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::HadContact))};
    TestTrue(TEXT("Same-Action A starts"), SameAction.StartA(Reason));
    TestTrue(TEXT("Same-Action contact records"),
        SameAction.Record(false, false, false, false, Reason));
    TestTrue(TEXT("Same-Action source reaches window"),
        SameAction.Sword->AdvanceRuntime(0.35f, Reason));
    TestTrue(TEXT("Same ActionId transition remains valid"), SameAction.RequestB(Reason));
    TestEqual(TEXT("Technique identity changes despite shared ActionId"),
        SameAction.Sword->GetActivePlan().TechniqueId, TechniqueB);
    TestEqual(TEXT("Shared ActionId remains unchanged"),
        SameAction.Sword->GetRuntimeState().ActionId, ActionA);
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FBufferedEvidenceTest, "BufferedEvidenceConsumption")
bool FBufferedEvidenceTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    Fixture.Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::AppliedDamage))};
    TestTrue(TEXT("A starts"), Fixture.StartA(Reason));
    const int64 SourceSerial = Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial();
    TestTrue(TEXT("A reaches bufferable time"),
        Fixture.Sword->AdvanceRuntime(0.22f, Reason));
    TestTrue(TEXT("Request buffers before evidence"), Fixture.RequestB(Reason));
    TestTrue(TEXT("Pending exists"), Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Pending creates no new outcome"),
        Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial(), SourceSerial);
    TestTrue(TEXT("Window opens without evidence"),
        Fixture.Sword->AdvanceRuntime(0.10f, Reason));
    TestTrue(TEXT("OutcomePending keeps intent alive"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("A remains active while outcome is pending"),
        Fixture.Sword->GetActivePlan().TechniqueId, TechniqueA);
    TestTrue(TEXT("Damage evidence records while pending"),
        Fixture.Record(true, false, false, false, Reason));
    TestTrue(TEXT("Zero-delta re-evaluation consumes"),
        Fixture.Sword->AdvanceRuntime(0.0f, Reason));
    TestFalse(TEXT("Consumed intent is cleared"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Consumption reason is explicit"),
        Fixture.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::Consumed);
    TestEqual(TEXT("A finalizes Transitioned"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeReason(),
        EKashmirCombatOutcomeFinalizationReason::Transitioned);
    TestEqual(TEXT("Finalized A retains source serial"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeExecutionSerial(), SourceSerial);
    TestTrue(TEXT("B receives a new serial"),
        Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial() > SourceSerial);
    TestEqual(TEXT("B starts with clean outcome"),
        Fixture.Sword->GetCurrentCombatOutcomeContactCount(), 0);
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FExpirationTest, "ExpirationPrecedesEligibility")
bool FExpirationTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    Fixture.Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::AppliedDamage))};
    TestTrue(TEXT("A starts"), Fixture.StartA(Reason));
    TestTrue(TEXT("A reaches bufferable time"),
        Fixture.Sword->AdvanceRuntime(0.22f, Reason));
    const int64 SourceSerial = Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial();
    TestTrue(TEXT("Request buffers"), Fixture.RequestB(Reason));
    TestTrue(TEXT("Evidence appears before large advance"),
        Fixture.Record(true, false, false, false, Reason));
    TestTrue(TEXT("Large advance remains operational"),
        Fixture.Sword->AdvanceRuntime(0.21f, Reason));
    TestFalse(TEXT("Expired pending cannot be resurrected"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Expiration wins over eligibility"),
        Fixture.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::Expired);
    TestEqual(TEXT("Expired request preserves A Technique"),
        Fixture.Sword->GetActivePlan().TechniqueId, TechniqueA);
    TestEqual(TEXT("Expired request preserves A serial"),
        Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial(), SourceSerial);
    TestFalse(TEXT("Expired request does not finalize A"),
        Fixture.Sword->HasLastFinalizedCombatOutcome());

    FFixture Never;
    Never.Initialize();
    Never.Style->TransitionRules = {MakeRule(
        TechniqueB, FactMask(EKashmirCombatOutcomeFact::AppliedDamage))};
    TestTrue(TEXT("No-evidence A starts"), Never.StartA(Reason));
    TestTrue(TEXT("No-evidence A reaches bufferable time"),
        Never.Sword->AdvanceRuntime(0.22f, Reason));
    TestTrue(TEXT("No-evidence request buffers"), Never.RequestB(Reason));
    TestTrue(TEXT("No-evidence request reaches window"),
        Never.Sword->AdvanceRuntime(0.10f, Reason));
    TestTrue(TEXT("Missing requirement remains pending in the window"),
        Never.Sword->HasPendingTechniqueRequest());
    TestTrue(TEXT("No-evidence request advances beyond lifetime"),
        Never.Sword->AdvanceRuntime(0.11f, Reason));
    TestFalse(TEXT("Requirement that never appears cannot start B"),
        Never.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("A remains active when requirement never appears"),
        Never.Sword->GetActivePlan().TechniqueId, TechniqueA);
    return true;
}


KASHMIR_OUTCOME_CONDITION_TEST(FNoEvidenceAndSourceIdentityTest, "NoMissAndSourceIdentity")
bool FNoEvidenceAndSourceIdentityTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    Fixture.Style->TransitionRules = {
        MakeRule(TechniqueB, FactMask(EKashmirCombatOutcomeFact::HadContact)),
        MakeRule(TechniqueC, 0, 0.20f, 0.30f)};
    TestTrue(TEXT("A starts"), Fixture.StartA(Reason));
    TestFalse(TEXT("Active no-contact state is not a satisfied HadContact fact"),
        Fixture.Sword->GetCurrentExecutionOutcome().bHadContact);
    TestTrue(TEXT("A reaches overlap of pending B and immediate C"),
        Fixture.Sword->AdvanceRuntime(0.22f, Reason));
    TestTrue(TEXT("B buffers without treating no-contact as Miss"),
        Fixture.RequestB(Reason));
    const int64 SourceSerial = Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial();
    TestTrue(TEXT("Unconditioned C transitions"),
        Fixture.Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot3), Fixture.Style, Reason));
    TestFalse(TEXT("Source change clears stale pending"),
        Fixture.Sword->HasPendingTechniqueRequest());
    TestEqual(TEXT("Stale pending clear reason is SourceChanged"),
        Fixture.Sword->GetLastPendingTechniqueClearReason(),
        EKashmirPendingTechniqueClearReason::SourceChanged);
    TestTrue(TEXT("New execution has a distinct serial"),
        Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial() > SourceSerial);
    TestEqual(TEXT("New execution cannot inherit stale facts"),
        Fixture.Sword->GetCurrentCombatOutcomeContactCount(), 0);
    return true;
}


}


#undef KASHMIR_OUTCOME_CONDITION_TEST

#endif
