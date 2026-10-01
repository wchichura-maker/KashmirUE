#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatOutcomeEvidence.h"
#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"


namespace KashmirCombatOutcomeEvidenceTests
{
    const FName TechniqueA(TEXT("Technique.Sword.Test.OutcomeA"));
    const FName TechniqueB(TEXT("Technique.Sword.Test.OutcomeB"));
    const FName ActionA(TEXT("Sword.Test.OutcomeA"));
    const FName ActionB(TEXT("Sword.Test.OutcomeB"));

    FKashmirSwordAuthoredAction MakeAction(
        const FName ActionId,
        const bool bCancellable = false,
        const bool bCostly = false)
    {
        FKashmirSwordAuthoredAction Result;
        Result.ActionId = ActionId;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_Outcome_Test.AM_Outcome_Test")));
        Result.StartupDuration = 0.18f;
        Result.ActiveDuration = 0.14f;
        Result.RecoveryDuration = 0.28f;
        Result.bCancellable = bCancellable;
        if (bCancellable)
        {
            Result.CancelWindows = {EKashmirActionPhase::Active};
        }
        if (bCostly)
        {
            FKashmirResourceCost Cost;
            Cost.ResourceTag = FGameplayTag::RequestGameplayTag(TEXT("Resource.Stamina"));
            Cost.Amount = 10.0f;
            Result.StartCosts.Add(Cost);
        }
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        return Result;
    }

    FKashmirCombatTechniqueDefinition MakeTechnique(
        const FName TechniqueId,
        const FName ActionId,
        const bool bCancellable = false,
        const bool bCostly = false)
    {
        FKashmirCombatTechniqueDefinition Result;
        Result.TechniqueId = TechniqueId;
        Result.ActionId = ActionId;
        Result.WeaponFamily = TEXT("Sword");
        Result.TechniqueFamily = TEXT("OutcomeTest");
        Result.AttackDirection = EKashmirAttackDirection::LeftToRight;
        Result.AttackShape = EKashmirAttackShape::Slash;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_Outcome_Test.AM_Outcome_Test")));
        Result.RuntimeDefinition =
            MakeAction(ActionId, bCancellable, bCostly).BuildRuntimeDefinition();
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Result.BaseDamage = 10.0f;
        return Result;
    }

    FKashmirTechniqueRequest MakeRequest(const EKashmirTechniqueSlot Slot)
    {
        FKashmirTechniqueRequest Result;
        Result.Slot = Slot;
        Result.Source = EKashmirTechniqueRequestSource::Player;
        return Result;
    }

    FKashmirCombatOutcomeContact MakeContact(
        const FName TargetId,
        const float Damage = 0.0f,
        const bool bBlocked = false,
        const bool bParried = false,
        const bool bGuardBroken = false)
    {
        FKashmirCombatOutcomeContact Result;
        Result.TargetId = TargetId;
        Result.bDamageApplied = Damage > 0.0f;
        Result.DamageApplied = Damage;
        Result.bBlocked = bBlocked;
        Result.bParried = bParried;
        Result.bGuardBroken = bGuardBroken;
        Result.CombatResult.bDelivered = true;
        Result.CombatResult.TargetIds.Add(TargetId);
        return Result;
    }

    struct FFixture
    {
        UKashmirDirectionalSwordProfile* Profile = nullptr;
        UKashmirWeaponCombatStyle* Style = nullptr;
        UKashmirDirectionalSwordComponent* Sword = nullptr;

        void Initialize(
            const bool bSameActionId = false,
            const bool bCancellable = false,
            const bool bCostlyDestination = false)
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
                Profile->Actions.Add(MakeAction(ActionB, false, bCostlyDestination));
                FKashmirSwordActionBinding ActionBindingB;
                ActionBindingB.Family = EKashmirSwordGestureFamily::Direct;
                ActionBindingB.Direction = EKashmirSwordGestureDirection::Left;
                ActionBindingB.ActionId = ActionB;
                Profile->GestureConfig.ActionBindings.Add(ActionBindingB);
            }

            Style = NewObject<UKashmirWeaponCombatStyle>();
            Style->StyleId = TEXT("Style.Sword.OutcomeTest");
            Style->WeaponFamily = TEXT("Sword");
            Style->Techniques.Add(MakeTechnique(
                TechniqueA, ActionA, bCancellable));
            Style->Techniques.Add(MakeTechnique(
                TechniqueB,
                bSameActionId ? ActionA : ActionB,
                false,
                bCostlyDestination));

            FKashmirTechniqueSlotBinding BindingA;
            BindingA.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
            BindingA.TechniqueId = TechniqueA;
            Style->SlotBindings.Add(BindingA);
            FKashmirTechniqueSlotBinding BindingB;
            BindingB.Slot = EKashmirTechniqueSlot::TechniqueSlot2;
            BindingB.TechniqueId = TechniqueB;
            Style->SlotBindings.Add(BindingB);

            FKashmirTechniqueTransitionRule Rule;
            Rule.FromTechniqueId = TechniqueA;
            Rule.ToTechniqueId = TechniqueB;
            Rule.MinElapsed = 0.32f;
            Rule.MaxElapsed = 0.47f;
            Style->TransitionRules.Add(Rule);

            Sword = NewObject<UKashmirDirectionalSwordComponent>();
            Sword->SetProfile(Profile);
        }

        bool StartA(FString& OutReason)
        {
            return Sword->StartTechniqueRequest(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), Style, OutReason);
        }

        bool RequestB(FString& OutReason)
        {
            return Sword->StartTechniqueRequest(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Style, OutReason);
        }

        bool Record(
            const FKashmirCombatOutcomeContact& Contact,
            FString& OutReason)
        {
            return Sword->RecordCombatOutcomeContact(Contact, OutReason);
        }
    };
}


#define KASHMIR_OUTCOME_TEST(TypeName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
        TypeName, \
        "Kashmir.Combat.CombatOutcomeEvidence." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


namespace KashmirCombatOutcomeEvidenceTests
{


KASHMIR_OUTCOME_TEST(FOutcomeAccumulatorTest, "ContactAccumulation")
bool FOutcomeAccumulatorTest::RunTest(const FString& Parameters)
{
    FKashmirCombatOutcomeAccumulator Accumulator;
    Accumulator.BeginExecution(7, TechniqueA, ActionA);
    TestTrue(TEXT("New execution exists"), Accumulator.HasCurrent());
    TestEqual(TEXT("New execution starts with zero contacts"),
        Accumulator.GetCurrent().ContactCount, 0);
    TestEqual(TEXT("New execution starts with zero targets"),
        Accumulator.GetCurrent().UniqueTargetCount, 0);

    FString Reason;
    TestTrue(TEXT("First authoritative contact records"),
        Accumulator.RecordContact(MakeContact(TEXT("Target.A"), 12.0f), Reason));
    TestTrue(TEXT("Duplicate target contact records"),
        Accumulator.RecordContact(
            MakeContact(TEXT("Target.A"), 3.0f, true), Reason));
    TestTrue(TEXT("Distinct target contact records"),
        Accumulator.RecordContact(
            MakeContact(TEXT("Target.B"), 0.0f, false, true, true), Reason));

    const FKashmirCombatExecutionOutcome& Outcome = Accumulator.GetCurrent();
    TestEqual(TEXT("Contacts count processed results, not sweeps"), Outcome.ContactCount, 3);
    TestEqual(TEXT("Duplicate target does not increase unique count"),
        Outcome.UniqueTargetCount, 2);
    TestEqual(TEXT("Actual applied damage accumulates"),
        Outcome.TotalDamageApplied, 15.0f);
    TestTrue(TEXT("Damage evidence accumulates"), Outcome.bAppliedDamage);
    TestTrue(TEXT("Block evidence accumulates"), Outcome.bWasBlocked);
    TestTrue(TEXT("Parry evidence accumulates"), Outcome.bWasParried);
    TestTrue(TEXT("Guard-break evidence accumulates"), Outcome.bCausedGuardBreak);
    TestEqual(TEXT("Last target is explicit"), Outcome.LastTargetId, FName(TEXT("Target.B")));
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeCompletionTest, "CompletionAndCleanRestart")
bool FOutcomeCompletionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    TestTrue(TEXT("A starts"), Fixture.StartA(Reason));
    const int64 FirstSerial = Fixture.Sword->GetCurrentExecutionOutcome().ExecutionSerial;
    TestTrue(TEXT("A contact records"),
        Fixture.Record(MakeContact(TEXT("Target.A"), 8.0f), Reason));
    TestTrue(TEXT("A completes"), Fixture.Sword->AdvanceRuntime(0.60f, Reason));
    TestFalse(TEXT("Completed execution has no current outcome"),
        Fixture.Sword->HasCurrentExecutionOutcome());
    const FKashmirCombatExecutionOutcome Final =
        Fixture.Sword->GetLastFinalizedExecutionOutcome();
    TestEqual(TEXT("Completion finalizes A"), Final.TechniqueId, TechniqueA);
    TestEqual(TEXT("Completion reason is explicit"), Final.FinalizationReason,
        EKashmirCombatOutcomeFinalizationReason::Completed);
    TestEqual(TEXT("Completed evidence is retained"), Final.ContactCount, 1);

    TestTrue(TEXT("A restarts"), Fixture.StartA(Reason));
    const FKashmirCombatExecutionOutcome Restart =
        Fixture.Sword->GetCurrentExecutionOutcome();
    TestTrue(TEXT("Restart receives a new serial"), Restart.ExecutionSerial > FirstSerial);
    TestEqual(TEXT("Restart does not inherit contacts"), Restart.ContactCount, 0);
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeZeroAndCancelTest, "ZeroContactAndCancellation")
bool FOutcomeZeroAndCancelTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Zero;
    Zero.Initialize();
    TestTrue(TEXT("Zero-contact A starts"), Zero.StartA(Reason));
    TestTrue(TEXT("Zero-contact A completes"), Zero.Sword->AdvanceRuntime(0.60f, Reason));
    TestEqual(TEXT("Zero-contact completion remains zero"),
        Zero.Sword->GetLastFinalizedExecutionOutcome().ContactCount, 0);

    FFixture Cancel;
    Cancel.Initialize(false, true);
    TestTrue(TEXT("Cancellable A starts"), Cancel.StartA(Reason));
    TestTrue(TEXT("A reaches cancellable Active phase"),
        Cancel.Sword->AdvanceRuntime(0.22f, Reason));
    TestTrue(TEXT("Cancellation succeeds"), Cancel.Sword->CancelCurrentAction(Reason));
    TestEqual(TEXT("Cancellation finalizes as Interrupted"),
        Cancel.Sword->GetLastFinalizedExecutionOutcome().FinalizationReason,
        EKashmirCombatOutcomeFinalizationReason::Interrupted);
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeTransitionTest, "ImmediateAndSameActionTransition")
bool FOutcomeTransitionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Immediate;
    Immediate.Initialize();
    TestTrue(TEXT("Immediate source starts"), Immediate.StartA(Reason));
    TestTrue(TEXT("Source evidence records"),
        Immediate.Record(MakeContact(TEXT("Target.A"), 9.0f), Reason));
    TestTrue(TEXT("Source reaches transition window"),
        Immediate.Sword->AdvanceRuntime(0.32f, Reason));
    TestTrue(TEXT("Immediate transition succeeds"), Immediate.RequestB(Reason));
    const FKashmirCombatExecutionOutcome FinalA =
        Immediate.Sword->GetLastFinalizedExecutionOutcome();
    const FKashmirCombatExecutionOutcome CurrentB =
        Immediate.Sword->GetCurrentExecutionOutcome();
    TestEqual(TEXT("A finalizes as Transitioned"), FinalA.FinalizationReason,
        EKashmirCombatOutcomeFinalizationReason::Transitioned);
    TestEqual(TEXT("A retains its evidence"), FinalA.ContactCount, 1);
    TestEqual(TEXT("B identity is current"), CurrentB.TechniqueId, TechniqueB);
    TestEqual(TEXT("B starts empty"), CurrentB.ContactCount, 0);
    TestTrue(TEXT("B has a distinct execution serial"),
        CurrentB.ExecutionSerial > FinalA.ExecutionSerial);

    FFixture SameAction;
    SameAction.Initialize(true);
    TestTrue(TEXT("Same-Action A starts"), SameAction.StartA(Reason));
    TestTrue(TEXT("Same-Action A records evidence"),
        SameAction.Record(MakeContact(TEXT("Target.A"), 5.0f), Reason));
    TestTrue(TEXT("Same-Action A reaches window"),
        SameAction.Sword->AdvanceRuntime(0.32f, Reason));
    TestTrue(TEXT("Same-Action Technique transition succeeds"),
        SameAction.RequestB(Reason));
    TestEqual(TEXT("Same ActionId source finalized separately"),
        SameAction.Sword->GetLastFinalizedExecutionOutcome().TechniqueId, TechniqueA);
    TestEqual(TEXT("Same ActionId destination is clean"),
        SameAction.Sword->GetCurrentExecutionOutcome().ContactCount, 0);
    TestEqual(TEXT("Same ActionId remains identical"),
        SameAction.Sword->GetCurrentExecutionOutcome().ActionId, ActionA);
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeBufferedTransitionTest, "BufferedTransition")
bool FOutcomeBufferedTransitionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    TestTrue(TEXT("Buffered source starts"), Fixture.StartA(Reason));
    TestTrue(TEXT("Buffered source records contact"),
        Fixture.Record(MakeContact(TEXT("Target.A"), 7.0f), Reason));
    TestTrue(TEXT("Source advances before window"),
        Fixture.Sword->AdvanceRuntime(0.22f, Reason));
    const int64 SourceSerial =
        Fixture.Sword->GetCurrentExecutionOutcome().ExecutionSerial;
    TestTrue(TEXT("Early B request buffers"), Fixture.RequestB(Reason));
    TestTrue(TEXT("Pending request does not create a new outcome"),
        Fixture.Sword->GetCurrentExecutionOutcome().ExecutionSerial == SourceSerial);
    TestEqual(TEXT("Pending request leaves A evidence intact"),
        Fixture.Sword->GetCurrentExecutionOutcome().ContactCount, 1);
    TestTrue(TEXT("Advance consumes buffer"),
        Fixture.Sword->AdvanceRuntime(0.10f, Reason));
    TestEqual(TEXT("Buffered source finalizes as Transitioned"),
        Fixture.Sword->GetLastFinalizedExecutionOutcome().FinalizationReason,
        EKashmirCombatOutcomeFinalizationReason::Transitioned);
    TestEqual(TEXT("Buffered destination starts clean"),
        Fixture.Sword->GetCurrentExecutionOutcome().ContactCount, 0);
    TestTrue(TEXT("Buffered B records only B evidence"),
        Fixture.Record(MakeContact(TEXT("Target.B"), 4.0f), Reason));
    TestEqual(TEXT("B owns only its own target"),
        Fixture.Sword->GetCurrentExecutionOutcome().LastTargetId,
        FName(TEXT("Target.B")));
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeFailedTransitionTest, "FailedTransitionIsNonDestructive")
bool FOutcomeFailedTransitionTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize(false, false, true);
    TestTrue(TEXT("Source starts"), Fixture.StartA(Reason));
    TestTrue(TEXT("Source evidence records"),
        Fixture.Record(MakeContact(TEXT("Target.A"), 6.0f), Reason));
    TestTrue(TEXT("Source reaches window"), Fixture.Sword->AdvanceRuntime(0.32f, Reason));
    const int64 SourceSerial =
        Fixture.Sword->GetCurrentExecutionOutcome().ExecutionSerial;
    TestFalse(TEXT("Unpayable destination transition fails"), Fixture.RequestB(Reason));
    TestTrue(TEXT("Source outcome remains current"),
        Fixture.Sword->HasCurrentExecutionOutcome());
    TestEqual(TEXT("Failed transition preserves serial"),
        Fixture.Sword->GetCurrentExecutionOutcome().ExecutionSerial, SourceSerial);
    TestEqual(TEXT("Failed transition preserves evidence"),
        Fixture.Sword->GetCurrentExecutionOutcome().ContactCount, 1);
    TestFalse(TEXT("Failed transition does not finalize source"),
        Fixture.Sword->HasLastFinalizedExecutionOutcome());
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeScalarObservabilityTest, "ScalarDebugProjection")
bool FOutcomeScalarObservabilityTest::RunTest(const FString& Parameters)
{
    FString Reason;
    FFixture Fixture;
    Fixture.Initialize();
    TestTrue(TEXT("Source starts"), Fixture.StartA(Reason));
    TestTrue(TEXT("Source contact records"),
        Fixture.Record(MakeContact(TEXT("Target.Debug"), 11.0f), Reason));

    const int64 SourceSerial = Fixture.Sword->GetCurrentCombatOutcomeExecutionSerial();
    TestTrue(TEXT("Current scalar projection exists"),
        Fixture.Sword->HasCurrentCombatOutcome());
    TestTrue(TEXT("Current serial is exposed"), SourceSerial > 0);
    TestEqual(TEXT("Current TechniqueId is exposed"),
        Fixture.Sword->GetCurrentCombatOutcomeTechniqueId(), TechniqueA);
    TestEqual(TEXT("Current ActionId is exposed"),
        Fixture.Sword->GetCurrentCombatOutcomeActionId(), ActionA);
    TestEqual(TEXT("Current contact count is exposed"),
        Fixture.Sword->GetCurrentCombatOutcomeContactCount(), 1);
    TestEqual(TEXT("Current unique-target count is exposed"),
        Fixture.Sword->GetCurrentCombatOutcomeUniqueTargetCount(), 1);
    TestEqual(TEXT("Current damage is exposed"),
        Fixture.Sword->GetCurrentCombatOutcomeTotalDamageApplied(), 11.0f);
    TestEqual(TEXT("Current last target is exposed"),
        Fixture.Sword->GetCurrentCombatOutcomeLastTargetId(), FName(TEXT("Target.Debug")));
    TestEqual(TEXT("Repeated scalar reads do not mutate contacts"),
        Fixture.Sword->GetCurrentCombatOutcomeContactCount(), 1);

    TestTrue(TEXT("Source reaches transition window"),
        Fixture.Sword->AdvanceRuntime(0.32f, Reason));
    TestTrue(TEXT("Destination transition succeeds"), Fixture.RequestB(Reason));
    TestTrue(TEXT("Last finalized scalar projection exists"),
        Fixture.Sword->HasLastFinalizedCombatOutcome());
    TestEqual(TEXT("Last serial is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeExecutionSerial(), SourceSerial);
    TestEqual(TEXT("Last TechniqueId is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeTechniqueId(), TechniqueA);
    TestEqual(TEXT("Last ActionId is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeActionId(), ActionA);
    TestEqual(TEXT("Last contact count is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeContactCount(), 1);
    TestEqual(TEXT("Last unique-target count is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeUniqueTargetCount(), 1);
    TestEqual(TEXT("Last damage is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeTotalDamageApplied(), 11.0f);
    TestEqual(TEXT("Last finalization reason is exposed"),
        Fixture.Sword->GetLastFinalizedCombatOutcomeReason(),
        EKashmirCombatOutcomeFinalizationReason::Transitioned);
    TestEqual(TEXT("Destination current projection starts clean"),
        Fixture.Sword->GetCurrentCombatOutcomeContactCount(), 0);

    const FProperty* AppliedFlag =
        FKashmirCombatOutcomeContact::StaticStruct()->FindPropertyByName(
            GET_MEMBER_NAME_CHECKED(FKashmirCombatOutcomeContact, bDamageApplied));
    const FProperty* AppliedAmount =
        FKashmirCombatOutcomeContact::StaticStruct()->FindPropertyByName(
            GET_MEMBER_NAME_CHECKED(FKashmirCombatOutcomeContact, DamageApplied));
    TestNotNull(TEXT("Damage-applied flag remains reflected"), AppliedFlag);
    TestNotNull(TEXT("Damage amount remains reflected"), AppliedAmount);
    if (AppliedFlag && AppliedAmount)
    {
        const FString FlagScriptName = AppliedFlag->GetMetaData(TEXT("ScriptName"));
        const FString AmountScriptName = AppliedAmount->GetMetaData(TEXT("ScriptName"));
        TestEqual(TEXT("Flag receives an explicit scripting name"),
            FlagScriptName, FString(TEXT("did_apply_damage")));
        TestEqual(TEXT("Amount receives an explicit scripting name"),
            AmountScriptName, FString(TEXT("applied_damage_amount")));
        TestNotEqual(TEXT("Scripting names are distinct"),
            FlagScriptName, AmountScriptName);
    }

    static const FName RequiredFunctions[] = {
        TEXT("GetCurrentCombatOutcomeExecutionSerial"),
        TEXT("GetCurrentCombatOutcomeTechniqueId"),
        TEXT("GetCurrentCombatOutcomeContactCount"),
        TEXT("GetCurrentCombatOutcomeTotalDamageApplied"),
        TEXT("GetLastFinalizedCombatOutcomeExecutionSerial"),
        TEXT("GetLastFinalizedCombatOutcomeTechniqueId"),
        TEXT("GetLastFinalizedCombatOutcomeTotalDamageApplied"),
        TEXT("GetLastFinalizedCombatOutcomeReason")
    };
    for (const FName FunctionName : RequiredFunctions)
    {
        const UFunction* Function =
            UKashmirDirectionalSwordComponent::StaticClass()->FindFunctionByName(FunctionName);
        TestNotNull(*FString::Printf(TEXT("%s is reflected"), *FunctionName.ToString()),
            Function);
        if (Function)
        {
            TestTrue(*FString::Printf(TEXT("%s is BlueprintPure"), *FunctionName.ToString()),
                Function->HasAnyFunctionFlags(FUNC_BlueprintPure));
            TestTrue(*FString::Printf(TEXT("%s is DevelopmentOnly"), *FunctionName.ToString()),
                Function->HasMetaData(TEXT("DevelopmentOnly")));
        }
    }
    return true;
}


KASHMIR_OUTCOME_TEST(FOutcomeAuthorityTest, "AuthorityBoundaries")
bool FOutcomeAuthorityTest::RunTest(const FString& Parameters)
{
    FString RuntimeSource;
    FString TraceSource;
    FString CharacterSource;
    const FString Root = FPaths::ProjectDir();
    TestTrue(TEXT("ActionRuntime source is readable"), FFileHelper::LoadFileToString(
        RuntimeSource, *FPaths::Combine(Root,
            TEXT("Source/KashmirUE/Private/Runtime/KashmirActionRuntime.cpp"))));
    TestTrue(TEXT("WeaponTrace source is readable"), FFileHelper::LoadFileToString(
        TraceSource, *FPaths::Combine(Root,
            TEXT("Source/KashmirUE/Private/Combat/KashmirWeaponTraceComponent.cpp"))));
    TestTrue(TEXT("Character integration source is readable"), FFileHelper::LoadFileToString(
        CharacterSource, *FPaths::Combine(Root,
            TEXT("Source/KashmirUE/Private/KashmirCharacter.cpp"))));
    TestFalse(TEXT("ActionRuntime remains outcome agnostic"),
        RuntimeSource.Contains(TEXT("CombatOutcome")));
    TestFalse(TEXT("WeaponTrace remains acquisition-only"),
        TraceSource.Contains(TEXT("CombatOutcome")));
    TestTrue(TEXT("Integration uses actual applied magnitude"),
        CharacterSource.Contains(TEXT("Applied.AppliedMagnitude")));
    TestFalse(TEXT("Integration does not infer damage from health delta"),
        CharacterSource.Contains(TEXT("HealthBefore -")) ||
        CharacterSource.Contains(TEXT("HealthAfter -")));
    return true;
}


}


#undef KASHMIR_OUTCOME_TEST

#endif
