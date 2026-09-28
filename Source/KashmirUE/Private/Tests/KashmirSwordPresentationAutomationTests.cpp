#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirSwordPresentationComponent.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirSwordActionPlan MakePresentationPlan()
    {
        FKashmirSwordActionPlan Plan;
        Plan.bResolved = true;
        Plan.RuntimeDefinition.ActionId = TEXT("Sword.Direct.Right");
        Plan.RuntimeDefinition.StartupDuration = 0.20f;
        Plan.RuntimeDefinition.ActiveDuration = 0.15f;
        Plan.RuntimeDefinition.RecoveryDuration = 0.30f;
        Plan.Montage = TSoftObjectPtr<UAnimMontage>(
            FSoftObjectPath(TEXT("/Game/KashmirAct/Test/AM_Sword_Test.AM_Sword_Test")));
        Plan.PlayRate = 1.25f;
        return Plan;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordPresentationSyncMatrixTest,
    "Kashmir.Combat.DirectionalSword.Presentation.SyncMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordPresentationSyncMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirSwordPresentationSyncResolver Resolver;
    const FKashmirSwordActionPlan Plan = MakePresentationPlan();
    FKashmirActionRuntimeState RuntimeState;
    RuntimeState.ActionId = Plan.RuntimeDefinition.ActionId;
    RuntimeState.Phase = EKashmirActionPhase::Startup;
    RuntimeState.Elapsed = 0.10f;
    RuntimeState.bActive = true;
    FKashmirSwordPresentationState PresentationState;
    FKashmirSwordPresentationSyncResult Result;
    FString Reason;

    TestTrue(TEXT("Inactive presentation resolves"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));
    TestEqual(TEXT("Active runtime starts montage"), Result.Command, EKashmirSwordPresentationCommand::Play);
    TestEqual(TEXT("Runtime time maps through play rate"), Result.MontagePositionSeconds, 0.125f);

    PresentationState.bActive = true;
    PresentationState.ActionId = RuntimeState.ActionId;
    RuntimeState.Elapsed = 0.30f;
    RuntimeState.Phase = EKashmirActionPhase::Active;
    TestTrue(TEXT("Active presentation resolves"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));
    TestEqual(TEXT("Matching montage synchronizes"), Result.Command, EKashmirSwordPresentationCommand::Synchronize);
    TestEqual(TEXT("Active phase position is authoritative"), Result.MontagePositionSeconds, 0.375f);

    RuntimeState.bActive = false;
    RuntimeState.Phase = EKashmirActionPhase::Complete;
    TestTrue(TEXT("Completed runtime resolves"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));
    TestEqual(TEXT("Completed runtime stops montage"), Result.Command, EKashmirSwordPresentationCommand::Stop);

    PresentationState = {};
    TestTrue(TEXT("Inactive pair resolves"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));
    TestEqual(TEXT("Inactive pair does nothing"), Result.Command, EKashmirSwordPresentationCommand::None);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordPresentationRejectInvalidTest,
    "Kashmir.Combat.DirectionalSword.Presentation.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordPresentationRejectInvalidTest::RunTest(const FString& Parameters)
{
    FKashmirSwordPresentationSyncResolver Resolver;
    FKashmirSwordActionPlan Plan = MakePresentationPlan();
    FKashmirActionRuntimeState RuntimeState;
    RuntimeState.ActionId = Plan.RuntimeDefinition.ActionId;
    RuntimeState.bActive = true;
    FKashmirSwordPresentationState PresentationState;
    FKashmirSwordPresentationSyncResult Result;
    FString Reason;

    Plan.bResolved = false;
    TestFalse(TEXT("Unresolved plan is rejected"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));

    Plan = MakePresentationPlan();
    Plan.PlayRate = 0.0f;
    TestFalse(TEXT("Invalid play rate is rejected"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));

    Plan = MakePresentationPlan();
    RuntimeState.Elapsed = std::numeric_limits<float>::quiet_NaN();
    TestFalse(TEXT("Invalid runtime elapsed is rejected"), Resolver.Resolve(Plan, RuntimeState, PresentationState, Result, Reason));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordPresentationComponentMissingMeshTest,
    "Kashmir.Combat.DirectionalSword.Presentation.RequiresMesh",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordPresentationComponentMissingMeshTest::RunTest(const FString& Parameters)
{
    UKashmirSwordPresentationComponent* Component =
        NewObject<UKashmirSwordPresentationComponent>();
    FKashmirActionRuntimeState RuntimeState;
    RuntimeState.ActionId = TEXT("Sword.Direct.Right");
    RuntimeState.Phase = EKashmirActionPhase::Startup;
    RuntimeState.bActive = true;
    FString Reason;

    TestFalse(
        TEXT("Presentation without mesh is rejected"),
        Component->ApplyRuntimeState(MakePresentationPlan(), RuntimeState, Reason));
    TestFalse(TEXT("Missing mesh reports reason"), Reason.IsEmpty());
    return true;
}

#endif
