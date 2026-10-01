#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirMovementDeliveryComponent.h"
#include "Combat/KashmirSwordPresentationComponent.h"
#include "Combat/KashmirWeaponTraceComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


namespace
{
    const FName TechniqueA(TEXT("Technique.Sword.Test.TransitionA"));
    const FName TechniqueB(TEXT("Technique.Sword.Test.TransitionB"));
    const FName TechniqueC(TEXT("Technique.Sword.Test.TransitionC"));
    const FName ActionA(TEXT("Sword.Test.TransitionA"));
    const FName ActionB(TEXT("Sword.Test.TransitionB"));

    FKashmirCombatTechniqueDefinition MakeTechnique(
        const FName TechniqueId,
        const FName ActionId)
    {
        FKashmirCombatTechniqueDefinition Result;
        Result.TechniqueId = TechniqueId;
        Result.ActionId = ActionId;
        Result.WeaponFamily = TEXT("Sword");
        Result.TechniqueFamily = TEXT("TransitionTest");
        Result.AttackDirection = EKashmirAttackDirection::LeftToRight;
        Result.AttackShape = EKashmirAttackShape::Slash;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_Transition_Test.AM_Transition_Test")));
        Result.RuntimeDefinition.ActionId = ActionId;
        Result.RuntimeDefinition.ActiveDuration = 0.20f;
        Result.RuntimeDefinition.RecoveryDuration = 0.20f;
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Result.BaseDamage = 10.0f;
        return Result;
    }

    FKashmirSwordAuthoredAction MakeAction(const FName ActionId)
    {
        FKashmirSwordAuthoredAction Result;
        Result.ActionId = ActionId;
        Result.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(
            TEXT("/Game/KashmirAct/Test/AM_Transition_Test.AM_Transition_Test")));
        Result.StartupDuration = 0.0f;
        Result.ActiveDuration = 0.20f;
        Result.RecoveryDuration = 0.20f;
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        return Result;
    }

    UKashmirDirectionalSwordProfile* MakeProfile(const bool bSameActionId = false)
    {
        UKashmirDirectionalSwordProfile* Profile =
            NewObject<UKashmirDirectionalSwordProfile>();
        Profile->GestureConfig.MinimumDragDistance = 10.0f;
        Profile->GestureConfig.FullIntensityDistance = 100.0f;

        FKashmirSwordActionBinding BindingA;
        BindingA.Family = EKashmirSwordGestureFamily::Direct;
        BindingA.Direction = EKashmirSwordGestureDirection::Right;
        BindingA.ActionId = ActionA;
        Profile->GestureConfig.ActionBindings.Add(BindingA);
        Profile->Actions.Add(MakeAction(ActionA));

        if (!bSameActionId)
        {
            FKashmirSwordActionBinding BindingB;
            BindingB.Family = EKashmirSwordGestureFamily::Direct;
            BindingB.Direction = EKashmirSwordGestureDirection::Left;
            BindingB.ActionId = ActionB;
            Profile->GestureConfig.ActionBindings.Add(BindingB);
            Profile->Actions.Add(MakeAction(ActionB));
        }
        return Profile;
    }

    UKashmirWeaponCombatStyle* MakeStyle(const bool bSameActionId = false)
    {
        UKashmirWeaponCombatStyle* Style = NewObject<UKashmirWeaponCombatStyle>();
        Style->StyleId = TEXT("Style.Sword.TransitionTest");
        Style->WeaponFamily = TEXT("Sword");
        Style->Techniques.Add(MakeTechnique(TechniqueA, ActionA));
        Style->Techniques.Add(MakeTechnique(
            TechniqueB, bSameActionId ? ActionA : ActionB));

        FKashmirTechniqueSlotBinding BindingA;
        BindingA.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
        BindingA.TechniqueId = TechniqueA;
        Style->SlotBindings.Add(BindingA);
        FKashmirTechniqueSlotBinding BindingB;
        BindingB.Slot = EKashmirTechniqueSlot::TechniqueSlot2;
        BindingB.TechniqueId = TechniqueB;
        Style->SlotBindings.Add(BindingB);
        return Style;
    }

    FKashmirTechniqueTransitionRule MakeRule(
        const FName From = TechniqueA,
        const FName To = TechniqueB,
        const float Min = 0.10f,
        const float Max = 0.20f,
        const int32 Priority = 0)
    {
        FKashmirTechniqueTransitionRule Rule;
        Rule.FromTechniqueId = From;
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
        FKashmirTechniqueRequest Request;
        Request.Slot = Slot;
        Request.Source = Source;
        return Request;
    }

    struct FTransitionWorld
    {
        UWorld* World = nullptr;
        ACharacter* Character = nullptr;
        UKashmirDirectionalSwordComponent* Sword = nullptr;
        UKashmirMovementDeliveryComponent* Movement = nullptr;
        UKashmirWeaponTraceComponent* Trace = nullptr;
        USceneComponent* TracePoint = nullptr;
        AActor* Target = nullptr;

        bool Initialize()
        {
            const FName WorldName = MakeUniqueObjectName(
                nullptr, UWorld::StaticClass(), TEXT("KashmirTechniqueTransitionWorld"),
                EUniqueObjectNameOptions::GloballyUnique);
            FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
            World = UWorld::CreateWorld(
                EWorldType::Game, false, WorldName, GetTransientPackage());
            if (World == nullptr)
            {
                return false;
            }
            World->AddToRoot();
            Context.SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());

            Character = World->SpawnActor<ACharacter>(
                FVector::ZeroVector, FRotator::ZeroRotator);
            if (Character == nullptr)
            {
                return false;
            }

            Movement = NewObject<UKashmirMovementDeliveryComponent>(
                Character, TEXT("TransitionMovement"), RF_Transient);
            Character->AddInstanceComponent(Movement);
            Movement->RegisterComponentWithWorld(World);
            Trace = NewObject<UKashmirWeaponTraceComponent>(
                Character, TEXT("TransitionTrace"), RF_Transient);
            Character->AddInstanceComponent(Trace);
            Trace->RegisterComponentWithWorld(World);
            TracePoint = NewObject<USceneComponent>(
                Character, TEXT("TransitionTracePoint"), RF_Transient);
            Character->AddInstanceComponent(TracePoint);
            TracePoint->RegisterComponentWithWorld(World);
            TracePoint->SetWorldLocation(FVector::ZeroVector);
            FKashmirWeaponContactPointBinding TraceBinding;
            TraceBinding.Id = TEXT("Weapon_Tip");
            TraceBinding.Component = TracePoint;
            Trace->SetContactPointBindings({TraceBinding});
            Trace->SetIgnoredActor(Character);

            Sword = NewObject<UKashmirDirectionalSwordComponent>(
                Character, TEXT("TransitionSword"), RF_Transient);
            Character->AddInstanceComponent(Sword);
            Sword->RegisterComponentWithWorld(World);
            Sword->SetMovementDeliveryComponent(Movement);
            Sword->SetWeaponTraceComponent(Trace);

            Target = World->SpawnActor<AActor>(
                FVector(50.0, 0.0, 0.0), FRotator::ZeroRotator);
            UBoxComponent* Box = Target != nullptr
                ? NewObject<UBoxComponent>(Target, TEXT("TransitionTarget"), RF_Transient)
                : nullptr;
            if (Box == nullptr)
            {
                return false;
            }
            Target->SetRootComponent(Box);
            Target->AddInstanceComponent(Box);
            Box->SetBoxExtent(FVector(10.0, 20.0, 20.0));
            Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Box->SetCollisionObjectType(ECC_Pawn);
            Box->SetCollisionResponseToAllChannels(ECR_Ignore);
            Box->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
            Box->RegisterComponentWithWorld(World);
            Box->UpdateComponentToWorld();
            World->Tick(LEVELTICK_All, 0.0f);
            return true;
        }

        ~FTransitionWorld()
        {
            if (World != nullptr)
            {
                GEngine->DestroyWorldContext(World);
                World->DestroyWorld(false);
                World->RemoveFromRoot();
            }
        }
    };
}


#define KASHMIR_TRANSITION_TEST(TypeName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
        TypeName, \
        "Kashmir.Combat.TechniqueTransitionGrammar." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


KASHMIR_TRANSITION_TEST(FTechniqueTransitionValidationTest, "Validation")
bool FTechniqueTransitionValidationTest::RunTest(const FString& Parameters)
{
    FString Reason;
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    TestTrue(TEXT("Empty rules preserve a valid style"), Style->ValidateStyle(Reason));

    Style->TransitionRules = {MakeRule(TEXT("Missing.Source"), TechniqueB)};
    TestFalse(TEXT("Missing source is rejected"), Style->ValidateStyle(Reason));
    Style->TransitionRules = {MakeRule(TechniqueA, TEXT("Missing.Destination"))};
    TestFalse(TEXT("Missing destination is rejected"), Style->ValidateStyle(Reason));
    Style->TransitionRules = {MakeRule(TechniqueA, TechniqueA)};
    TestFalse(TEXT("Self transition is rejected in v0.1"), Style->ValidateStyle(Reason));
    Style->TransitionRules = {MakeRule(), MakeRule()};
    TestFalse(TEXT("Semantic duplicate is rejected"), Style->ValidateStyle(Reason));

    FKashmirTechniqueTransitionRule Contradictory = MakeRule();
    const FGameplayTag TransitionTag = FGameplayTag::RequestGameplayTag(
        TEXT("Action.Transition"));
    Contradictory.RequiredTags.AddTag(TransitionTag);
    Contradictory.BlockedTags.AddTag(TransitionTag);
    Style->TransitionRules = {Contradictory};
    TestFalse(TEXT("Contradictory tags are rejected"), Style->ValidateStyle(Reason));
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionEligibilityTest, "Eligibility")
bool FTechniqueTransitionEligibilityTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    FKashmirTechniqueTransitionRule Rule = MakeRule();
    const FGameplayTag Required = FGameplayTag::RequestGameplayTag(TEXT("Action.Transition"));
    const FGameplayTag Blocked = FGameplayTag::RequestGameplayTag(TEXT("State.Disabled"));
    Rule.RequiredTags.AddTag(Required);
    Rule.BlockedTags.AddTag(Blocked);
    Style->TransitionRules = {Rule};

    FString Reason;
    FKashmirTechniqueTransitionRule Resolved;
    FGameplayTagContainer Context;
    Context.AddTag(Required);
    TestFalse(TEXT("Before window is rejected"), Style->ResolveTechniqueTransition(
        TechniqueA, TechniqueB, 0.09f, Context, Resolved, Reason));
    TestTrue(TEXT("Inclusive window is accepted"), Style->ResolveTechniqueTransition(
        TechniqueA, TechniqueB, 0.10f, Context, Resolved, Reason));
    TestFalse(TEXT("After window is rejected"), Style->ResolveTechniqueTransition(
        TechniqueA, TechniqueB, 0.21f, Context, Resolved, Reason));
    Context.Reset();
    TestFalse(TEXT("Missing required tag is rejected"), Style->ResolveTechniqueTransition(
        TechniqueA, TechniqueB, 0.15f, Context, Resolved, Reason));
    Context.AddTag(Required);
    Context.AddTag(Blocked);
    TestFalse(TEXT("Blocked tag suppresses transition"), Style->ResolveTechniqueTransition(
        TechniqueA, TechniqueB, 0.15f, Context, Resolved, Reason));
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionOrderingTest, "PriorityOrdering")
bool FTechniqueTransitionOrderingTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    Style->Techniques.Add(MakeTechnique(TechniqueC, TEXT("Sword.Test.TransitionC")));
    Style->TransitionRules = {
        MakeRule(TechniqueA, TechniqueB, 0.0f, -1.0f, 1),
        MakeRule(TechniqueA, TechniqueB, 0.0f, 0.5f, 5),
        MakeRule(TechniqueA, TechniqueC, 0.0f, -1.0f, 3)};
    FString Reason;
    TestTrue(TEXT("Distinct rules validate"), Style->ValidateStyle(Reason));
    const TArray<FName> Options = Style->GetTechniqueTransitionOptions(
        TechniqueA, 0.1f, {});
    TestEqual(TEXT("Duplicate destinations are collapsed"), Options.Num(), 2);
    TestEqual(TEXT("Highest-priority destination sorts first"), Options[0], TechniqueB);
    TestEqual(TEXT("Second destination remains deterministic"), Options[1], TechniqueC);
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionRuntimeTest, "RuntimeSemantics")
bool FTechniqueTransitionRuntimeTest::RunTest(const FString& Parameters)
{
    const FGameplayTag Stamina = FGameplayTag::RequestGameplayTag(TEXT("Resource.Stamina"));
    FKashmirResourcePool Pool;
    Pool.ResourceTag = Stamina;
    Pool.Current = 5.0f;
    Pool.Maximum = 5.0f;
    FKashmirResourceRuntime Resources({Pool});
    FKashmirActionDefinition A;
    A.ActionId = ActionA;
    A.ActiveDuration = 0.2f;
    A.RecoveryDuration = 0.2f;
    A.bCancellable = true;
    A.CancelWindows = {EKashmirActionPhase::Active};
    FKashmirActionDefinition B = A;
    B.ActionId = ActionB;
    FKashmirResourceCost Cost;
    Cost.ResourceTag = Stamina;
    Cost.Amount = 10.0f;
    B.StartCosts = {Cost};
    TMap<FName, FKashmirActionDefinition> Definitions;
    Definitions.Add(ActionA, A);
    Definitions.Add(ActionB, B);
    FKashmirActionRuntime Runtime(Definitions, {}, Resources);
    FKashmirActionRequest RequestA;
    RequestA.ActionId = ActionA;
    FKashmirActionRequest RequestB;
    RequestB.ActionId = ActionB;
    FKashmirTransitionRule Rule;
    Rule.FromActionId = ActionA;
    Rule.ToActionId = ActionB;
    Rule.MinElapsed = 0.0f;
    Rule.MaxElapsed = 0.2f;
    FString Reason;
    TestTrue(TEXT("Source starts"), Runtime.Start(RequestA, Reason));
    Runtime.DrainEvents();
    TestFalse(TEXT("Resource preflight rejects destination"), Runtime.CanTransitionTo(
        RequestB, Rule, {}, Reason));
    TestEqual(TEXT("Resource failure keeps source action"), Runtime.GetState().ActionId, ActionA);
    TestTrue(TEXT("Resource failure keeps source active"), Runtime.GetState().bActive);
    TestTrue(TEXT("Resource failure emits no event"), Runtime.DrainEvents().IsEmpty());

    B.StartCosts.Reset();
    FKashmirResourceRuntime FreeResources({Pool});
    Definitions[ActionB] = B;
    FKashmirActionRuntime TransitionRuntime(Definitions, {}, FreeResources);
    TestTrue(TEXT("Transition source starts"), TransitionRuntime.Start(RequestA, Reason));
    TransitionRuntime.DrainEvents();
    TestTrue(TEXT("Authorized transition commits"), TransitionRuntime.TransitionTo(
        RequestB, Rule, {}, Reason));
    const TArray<FKashmirActionEvent> Events = TransitionRuntime.DrainEvents();
    TestEqual(TEXT("Transition emits two ordered events"), Events.Num(), 2);
    TestEqual(TEXT("First event is Transitioned"), Events[0].Type,
        EKashmirActionEventType::Transitioned);
    TestEqual(TEXT("Second event is Started"), Events[1].Type,
        EKashmirActionEventType::Started);

    FKashmirActionRuntime CancelRuntime(Definitions, {}, FreeResources);
    TestTrue(TEXT("Cancel source starts"), CancelRuntime.Start(RequestA, Reason));
    CancelRuntime.DrainEvents();
    TestTrue(TEXT("Cancel replacement remains available"), CancelRuntime.TryCancel(
        RequestB, Reason));
    const TArray<FKashmirActionEvent> CancelEvents = CancelRuntime.DrainEvents();
    TestEqual(TEXT("Cancel remains Interrupted"), CancelEvents[0].Type,
        EKashmirActionEventType::Interrupted);
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionSwordTest, "SwordOrchestration")
bool FTechniqueTransitionSwordTest::RunTest(const FString& Parameters)
{
    FTransitionWorld Fixture;
    if (!TestTrue(TEXT("Transition world initializes"), Fixture.Initialize()))
    {
        return false;
    }
    UKashmirDirectionalSwordProfile* Profile = MakeProfile();
    Profile->Actions[0].StartupDuration = 0.0f;
    Profile->Actions[1].StartupDuration = 0.0f;
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    Style->TransitionRules = {MakeRule()};
    Style->Techniques[0].MovementSpec.Delivery =
        EKashmirMovementDelivery::ControlledTranslation;
    Style->Techniques[0].MovementSpec.Distance = 20.0f;
    Style->Techniques[0].MovementSpec.Duration = 0.20f;
    Style->Techniques[1].MovementSpec.Delivery =
        EKashmirMovementDelivery::ControlledTranslation;
    Style->Techniques[1].MovementSpec.Distance = 10.0f;
    Style->Techniques[1].MovementSpec.Duration = 0.20f;
    Fixture.Sword->SetProfile(Profile);

    FString Reason;
    TestTrue(TEXT("Technique A starts"), Fixture.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), Style, Reason));
    Fixture.Sword->DrainRuntimeEvents();
    TestTrue(TEXT("A movement delivery starts"), Fixture.Movement->IsDeliveryActive());
    TestTrue(TEXT("A active phase opens trace"), Fixture.Trace->IsTraceWindowActive());
    const int32 FirstGeneration = Fixture.Trace->GetTraceWindowGenerationForDebug();

    TArray<FKashmirWeaponTraceHit> Hits;
    TestTrue(TEXT("A trace captures initial frame"), Fixture.Trace->SampleTrace(
        0.01f, Hits, Reason));
    Fixture.TracePoint->SetWorldLocation(FVector(100.0f, 0.0f, 0.0f));
    TestTrue(TEXT("A trace sweeps through target"), Fixture.Trace->SampleTrace(
        0.10f, Hits, Reason));
    TestTrue(TEXT("A records target in its hit set"),
        Fixture.Trace->GetHitActorCountForDebug() > 0);

    TestTrue(TEXT("Runtime advances into authored window"),
        Fixture.Sword->AdvanceRuntime(0.10f, Reason));
    TestTrue(TEXT("Technique B transitions"), Fixture.Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Style, Reason));
    TestEqual(TEXT("Active plan becomes destination Technique"),
        Fixture.Sword->GetActivePlan().TechniqueId, TechniqueB);
    TestEqual(TEXT("Destination Action becomes active"),
        Fixture.Sword->GetRuntimeState().ActionId, ActionB);
    TestEqual(TEXT("Previous delivery ends as Transitioned"),
        Fixture.Movement->GetLastCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Transitioned);
    TestTrue(TEXT("Destination delivery starts normally"),
        Fixture.Movement->IsDeliveryActive());
    TestTrue(TEXT("Destination trace window opens independently"),
        Fixture.Trace->IsTraceWindowActive());
    TestEqual(TEXT("Destination trace uses a new generation"),
        Fixture.Trace->GetTraceWindowGenerationForDebug(), FirstGeneration + 1);
    TestEqual(TEXT("Previous hit set does not leak"),
        Fixture.Trace->GetHitActorCountForDebug(), 0);
    const TArray<FKashmirActionEvent> Events = Fixture.Sword->DrainRuntimeEvents();
    TestEqual(TEXT("Sword transition emits two events"), Events.Num(), 2);
    TestEqual(TEXT("Sword transition does not interrupt A"), Events[0].Type,
        EKashmirActionEventType::Transitioned);
    TestEqual(TEXT("Sword destination starts second"), Events[1].Type,
        EKashmirActionEventType::Started);
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionSameActionTest, "SameActionIdentity")
bool FTechniqueTransitionSameActionTest::RunTest(const FString& Parameters)
{
    UKashmirDirectionalSwordComponent* Sword =
        NewObject<UKashmirDirectionalSwordComponent>();
    Sword->SetProfile(MakeProfile(true));
    UKashmirWeaponCombatStyle* Style = MakeStyle(true);
    Style->TransitionRules = {MakeRule()};
    FString Reason;
    TestTrue(TEXT("First Technique sharing ActionId starts"),
        Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), Style, Reason));
    TestTrue(TEXT("Shared Action advances into window"),
        Sword->AdvanceRuntime(0.10f, Reason));
    TestTrue(TEXT("Different Technique with same ActionId transitions"),
        Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Style, Reason));
    TestEqual(TEXT("Technique identity changes"),
        Sword->GetActivePlan().TechniqueId, TechniqueB);
    TestEqual(TEXT("Action identity may remain shared"),
        Sword->GetRuntimeState().ActionId, ActionA);

    FKashmirSwordActionPlan Destination = Sword->GetActivePlan();
    FKashmirSwordPresentationState Presentation;
    Presentation.bActive = true;
    Presentation.ActionId = ActionA;
    Presentation.TechniqueId = TechniqueA;
    FKashmirSwordPresentationSyncResult Sync;
    FKashmirSwordPresentationSyncResolver Resolver;
    TestTrue(TEXT("Presentation resolves shared Action transition"), Resolver.Resolve(
        Destination, Sword->GetRuntimeState(), Presentation, true, Sync, Reason));
    TestEqual(TEXT("Technique identity forces presentation replacement"), Sync.Command,
        EKashmirSwordPresentationCommand::Play);
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionFallbackTest, "FallbackAndSources")
bool FTechniqueTransitionFallbackTest::RunTest(const FString& Parameters)
{
    UKashmirDirectionalSwordProfile* Profile = MakeProfile();
    Profile->Actions[0].bCancellable = true;
    Profile->Actions[0].CancelWindows = {EKashmirActionPhase::Active};
    UKashmirWeaponCombatStyle* Style = MakeStyle();
    UKashmirDirectionalSwordComponent* Sword =
        NewObject<UKashmirDirectionalSwordComponent>();
    Sword->SetProfile(Profile);
    FString Reason;
    TestTrue(TEXT("Empty-rule source starts"), Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), Style, Reason));
    Sword->DrainRuntimeEvents();
    TestTrue(TEXT("Empty rules preserve cancel replacement"),
        Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), Style, Reason));
    const TArray<FKashmirActionEvent> Events = Sword->DrainRuntimeEvents();
    TestEqual(TEXT("Fallback emits Interrupted"), Events[0].Type,
        EKashmirActionEventType::Interrupted);

    for (const EKashmirTechniqueRequestSource Source : {
            EKashmirTechniqueRequestSource::Player,
            EKashmirTechniqueRequestSource::AI,
            EKashmirTechniqueRequestSource::Replay,
            EKashmirTechniqueRequestSource::Network})
    {
        FKashmirTechniqueActionPlan Plan;
        TestTrue(TEXT("Request source does not alter style resolution"),
            Style->ResolveTechnique(
                MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1, Source),
                Plan,
                Reason));
        TestEqual(TEXT("Source metadata is preserved"),
            Plan.TechniqueRequest.Source, Source);
    }
    FKashmirTechniqueActionPlan Unbound;
    TestFalse(TEXT("Slot5 remains safely unbound"), Style->ResolveTechnique(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot5), Unbound, Reason));
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionBaselineProofTest, "Slot1ToSlot2Proof")
bool FTechniqueTransitionBaselineProofTest::RunTest(const FString& Parameters)
{
    constexpr const TCHAR* StylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/DA_KashmirSword_CombatStyle_Baseline.DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* ProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/DA_KashmirDirectionalSword_Baseline.DA_KashmirDirectionalSword_Baseline");
    const UKashmirWeaponCombatStyle* BaselineStyle =
        LoadObject<UKashmirWeaponCombatStyle>(nullptr, StylePath);
    UKashmirDirectionalSwordProfile* Profile =
        LoadObject<UKashmirDirectionalSwordProfile>(nullptr, ProfilePath);
    if (!TestNotNull(TEXT("Baseline style loads"), BaselineStyle) ||
        !TestNotNull(TEXT("Baseline profile loads"), Profile))
    {
        return false;
    }

    const FName FromTechniqueId(
        TEXT("Technique.Sword.Horizontal.LeftToRight"));
    const FName ToTechniqueId(
        TEXT("Technique.Sword.Diagonal.Rising.RightToLeft"));
    const FName StepForwardTechniqueId(
        TEXT("Technique.Sword.Test.StepForward"));
    FString Reason;
    TestTrue(TEXT("Persistent baseline style is valid"),
        BaselineStyle->ValidateStyle(Reason));
    TestEqual(TEXT("Persistent baseline retains eight Techniques"),
        BaselineStyle->Techniques.Num(), 8);
    TestEqual(TEXT("Persistent baseline retains four Slot bindings"),
        BaselineStyle->SlotBindings.Num(), 4);
    TestEqual(TEXT("Persistent baseline owns exactly one transition rule"),
        BaselineStyle->TransitionRules.Num(), 1);
    if (BaselineStyle->TransitionRules.Num() != 1)
    {
        return false;
    }

    const FKashmirTechniqueTransitionRule& PersistentRule =
        BaselineStyle->TransitionRules[0];
    TestEqual(TEXT("Persistent rule source is Slot1 Technique"),
        PersistentRule.FromTechniqueId, FromTechniqueId);
    TestEqual(TEXT("Persistent rule destination is Slot2 Technique"),
        PersistentRule.ToTechniqueId, ToTechniqueId);
    TestEqual(TEXT("Persistent rule minimum elapsed is 0.320 s"),
        PersistentRule.MinElapsed, 0.320f);
    TestEqual(TEXT("Persistent rule maximum elapsed is 0.470 s"),
        PersistentRule.MaxElapsed, 0.470f);
    TestEqual(TEXT("Persistent rule priority remains zero"),
        PersistentRule.Priority, 0);
    TestTrue(TEXT("Persistent rule requires no tags"),
        PersistentRule.RequiredTags.IsEmpty());
    TestTrue(TEXT("Persistent rule blocks no tags"),
        PersistentRule.BlockedTags.IsEmpty());

    const FKashmirCombatTechniqueDefinition* FromTechnique =
        BaselineStyle->Techniques.FindByPredicate(
            [&FromTechniqueId](const FKashmirCombatTechniqueDefinition& Technique)
            {
                return Technique.TechniqueId == FromTechniqueId;
            });
    const FKashmirCombatTechniqueDefinition* ToTechnique =
        BaselineStyle->Techniques.FindByPredicate(
            [&ToTechniqueId](const FKashmirCombatTechniqueDefinition& Technique)
            {
                return Technique.TechniqueId == ToTechniqueId;
            });
    const FKashmirCombatTechniqueDefinition* StepForward =
        BaselineStyle->Techniques.FindByPredicate(
            [&StepForwardTechniqueId](const FKashmirCombatTechniqueDefinition& Technique)
            {
                return Technique.TechniqueId == StepForwardTechniqueId;
            });
    TestNotNull(TEXT("Persistent rule source Technique exists"), FromTechnique);
    TestNotNull(TEXT("Persistent rule destination Technique exists"), ToTechnique);
    TestNotNull(TEXT("Persistent StepForward remains present"), StepForward);

    const FKashmirTechniqueSlotBinding* Slot1Binding =
        BaselineStyle->SlotBindings.FindByPredicate(
            [](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.Slot == EKashmirTechniqueSlot::TechniqueSlot1;
            });
    const FKashmirTechniqueSlotBinding* Slot2Binding =
        BaselineStyle->SlotBindings.FindByPredicate(
            [](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.Slot == EKashmirTechniqueSlot::TechniqueSlot2;
            });
    TestNotNull(TEXT("Slot1 binding remains present"), Slot1Binding);
    TestNotNull(TEXT("Slot2 binding remains present"), Slot2Binding);
    if (FromTechnique == nullptr || ToTechnique == nullptr ||
        StepForward == nullptr || Slot1Binding == nullptr || Slot2Binding == nullptr)
    {
        return false;
    }
    TestEqual(TEXT("Rule source matches the Slot1 binding"),
        PersistentRule.FromTechniqueId, Slot1Binding->TechniqueId);
    TestEqual(TEXT("Rule destination matches the Slot2 binding"),
        PersistentRule.ToTechniqueId, Slot2Binding->TechniqueId);
    TestFalse(TEXT("Slot5 remains unbound"),
        BaselineStyle->SlotBindings.ContainsByPredicate(
            [](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.Slot == EKashmirTechniqueSlot::TechniqueSlot5;
            }));
    TestFalse(TEXT("StepForward remains unbound"),
        BaselineStyle->SlotBindings.ContainsByPredicate(
            [&StepForwardTechniqueId](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.TechniqueId == StepForwardTechniqueId;
            }));
    TestTrue(TEXT("StepForward definition remains valid"),
        StepForward->IsValid(Reason));
    TestEqual(TEXT("StepForward retains Sword.Direct.Right"),
        StepForward->ActionId, FName(TEXT("Sword.Direct.Right")));
    TestEqual(TEXT("StepForward retains FullBody presentation"),
        StepForward->MovementIntent, EKashmirMovementIntent::FullBody);
    TestEqual(TEXT("StepForward retains ControlledTranslation"),
        StepForward->MovementSpec.Delivery,
        EKashmirMovementDelivery::ControlledTranslation);
    TestEqual(TEXT("StepForward retains 80 cm distance"),
        StepForward->MovementSpec.Distance, 80.0f);
    TestEqual(TEXT("StepForward retains 0.25 s duration"),
        StepForward->MovementSpec.Duration, 0.25f);
    TestEqual(TEXT("StepForward remains Actor-relative"),
        StepForward->MovementSpec.Reference, EKashmirMovementReference::Actor);
    TestEqual(TEXT("StepForward remains Forward"),
        StepForward->MovementSpec.Direction, EKashmirMovementDirection::Forward);
    TestEqual(TEXT("StepForward remains target-independent"),
        StepForward->MovementSpec.TargetPolicy,
        EKashmirMovementTargetPolicy::NotRequired);

    UKashmirWeaponCombatStyle* FixtureStyle = DuplicateObject<UKashmirWeaponCombatStyle>(
        BaselineStyle, GetTransientPackage());
    const FKashmirSwordAuthoredAction* SourceAction =
        Profile->FindAction(TEXT("Sword.Direct.Right"));
    if (!TestNotNull(TEXT("Slot1 authored Action exists"), SourceAction))
    {
        return false;
    }
    const float WindowStart = SourceAction->StartupDuration + SourceAction->ActiveDuration;
    const float WindowEnd = WindowStart + FMath::Min(0.15f, SourceAction->RecoveryDuration);
    TestEqual(TEXT("Persistent window begins at the end of source Active"),
        PersistentRule.MinElapsed, WindowStart);
    TestEqual(TEXT("Persistent window covers the approved Recovery interval"),
        PersistentRule.MaxElapsed, WindowEnd);
    AddInfo(FString::Printf(
        TEXT("Slot1 timeline startup=%.3f active=%.3f recovery=%.3f; proof window=[%.3f, %.3f]"),
        SourceAction->StartupDuration,
        SourceAction->ActiveDuration,
        SourceAction->RecoveryDuration,
        WindowStart,
        WindowEnd));

    UKashmirDirectionalSwordComponent* Sword =
        NewObject<UKashmirDirectionalSwordComponent>();
    Sword->SetProfile(Profile);
    TestTrue(TEXT("Baseline Slot1 starts"), Sword->StartTechniqueRequest(
        MakeRequest(EKashmirTechniqueSlot::TechniqueSlot1), FixtureStyle, Reason));
    TestTrue(TEXT("Baseline advances to end of Active"),
        Sword->AdvanceRuntime(WindowStart, Reason));
    TestTrue(TEXT("Fixture rule transitions Slot1 to Slot2"),
        Sword->StartTechniqueRequest(
            MakeRequest(EKashmirTechniqueSlot::TechniqueSlot2), FixtureStyle, Reason));
    TestEqual(TEXT("Destination is Slot2 Technique"),
        Sword->GetActivePlan().TechniqueId,
        ToTechniqueId);
    return true;
}


KASHMIR_TRANSITION_TEST(FTechniqueTransitionBoundaryTest, "AuthorityBoundaries")
bool FTechniqueTransitionBoundaryTest::RunTest(const FString& Parameters)
{
    FString RuntimeSource;
    const FString RuntimePath = FPaths::Combine(
        FPaths::ProjectDir(),
        TEXT("Source/KashmirUE/Private/Runtime/KashmirActionRuntime.cpp"));
    TestTrue(TEXT("ActionRuntime source is readable"),
        FFileHelper::LoadFileToString(RuntimeSource, *RuntimePath));
    TestFalse(TEXT("ActionRuntime contains no Technique-specific branch"),
        RuntimeSource.Contains(TEXT("TechniqueId")) ||
        RuntimeSource.Contains(TEXT("Technique.Sword")));

    FString DeliverySource;
    const FString DeliveryPath = FPaths::Combine(
        FPaths::ProjectDir(),
        TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"));
    TestTrue(TEXT("MovementDelivery source is readable"),
        FFileHelper::LoadFileToString(DeliverySource, *DeliveryPath));
    TestTrue(TEXT("CharacterMovement remains displacement authority"),
        DeliverySource.Contains(TEXT("SafeMoveUpdatedComponent")));
    TestFalse(TEXT("Transition grammar introduces no Root Motion authority"),
        DeliverySource.Contains(TEXT("RootMotion")));
    return true;
}

#undef KASHMIR_TRANSITION_TEST

#endif
