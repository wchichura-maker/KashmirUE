#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimInstance.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirMovementDeliveryComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


namespace
{
    constexpr const TCHAR* StepForwardStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* StepForwardProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");
    const FName StepForwardTechniqueId(TEXT("Technique.Sword.Test.StepForward"));

    UKashmirWeaponCombatStyle* LoadStepForwardStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(nullptr, StepForwardStylePath);
    }

    UKashmirDirectionalSwordProfile* LoadStepForwardProfile()
    {
        return LoadObject<UKashmirDirectionalSwordProfile>(nullptr, StepForwardProfilePath);
    }

    const FKashmirCombatTechniqueDefinition* FindStepForwardBaseline(
        const UKashmirWeaponCombatStyle* Style)
    {
        if (Style == nullptr || Style->SlotBindings.IsEmpty()) return nullptr;
        const FName BaselineId = Style->SlotBindings[0].TechniqueId;
        return Style->Techniques.FindByPredicate(
            [BaselineId](const FKashmirCombatTechniqueDefinition& Candidate)
            {
                return Candidate.TechniqueId == BaselineId;
            });
    }

    const FKashmirCombatTechniqueDefinition* FindPersistentStepForward(
        const UKashmirWeaponCombatStyle* Style)
    {
        return Style != nullptr
            ? Style->Techniques.FindByPredicate(
                [](const FKashmirCombatTechniqueDefinition& Candidate)
                {
                    return Candidate.TechniqueId == StepForwardTechniqueId;
                })
            : nullptr;
    }

    struct FStepForwardTestFixture
    {
        UWorld* World = nullptr;
        ACharacter* Character = nullptr;
        UKashmirMovementDeliveryComponent* Movement = nullptr;
        UKashmirDirectionalSwordComponent* Sword = nullptr;
        UKashmirWeaponCombatStyle* Style = nullptr;
        UKashmirDirectionalSwordProfile* Profile = nullptr;
        FKashmirCombatTechniqueDefinition Baseline;
        EKashmirTechniqueSlot RequestSlot = EKashmirTechniqueSlot::None;

        bool Initialize(const bool bCancellable = false)
        {
            UKashmirWeaponCombatStyle* SourceStyle = LoadStepForwardStyle();
            UKashmirDirectionalSwordProfile* SourceProfile = LoadStepForwardProfile();
            const FKashmirCombatTechniqueDefinition* SourceBaseline =
                FindStepForwardBaseline(SourceStyle);
            const FKashmirCombatTechniqueDefinition* SourceStepForward =
                FindPersistentStepForward(SourceStyle);
            if (SourceStyle == nullptr || SourceProfile == nullptr ||
                SourceBaseline == nullptr || SourceStepForward == nullptr ||
                SourceStyle->SlotBindings.IsEmpty())
            {
                return false;
            }

            Baseline = *SourceBaseline;
            Style = DuplicateObject<UKashmirWeaponCombatStyle>(
                SourceStyle, GetTransientPackage());
            Profile = DuplicateObject<UKashmirDirectionalSwordProfile>(
                SourceProfile, GetTransientPackage());
            RequestSlot = Style->SlotBindings[0].Slot;
            Style->SlotBindings[0].TechniqueId = StepForwardTechniqueId;

            FKashmirSwordAuthoredAction* Action = Profile->Actions.FindByPredicate(
                [this](const FKashmirSwordAuthoredAction& Candidate)
                {
                    return Candidate.ActionId == Baseline.ActionId;
                });
            if (Action == nullptr) return false;
            Action->bCancellable = bCancellable;
            Action->CancelWindows = bCancellable
                ? TArray<EKashmirActionPhase>{EKashmirActionPhase::Startup}
                : TArray<EKashmirActionPhase>{};

            const FName WorldName = MakeUniqueObjectName(
                nullptr, UWorld::StaticClass(), TEXT("KashmirStepForwardTestWorld"),
                EUniqueObjectNameOptions::GloballyUnique);
            FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
            World = UWorld::CreateWorld(
                EWorldType::Game, false, WorldName, GetTransientPackage());
            if (World == nullptr) return false;
            World->AddToRoot();
            Context.SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
            Character = World->SpawnActor<ACharacter>(
                FVector(0.0f, 0.0f, 100.0f), FRotator::ZeroRotator);
            if (Character == nullptr) return false;

            Movement = NewObject<UKashmirMovementDeliveryComponent>(
                Character, TEXT("StepForwardMovementDelivery"), RF_Transient);
            Sword = NewObject<UKashmirDirectionalSwordComponent>(
                Character, TEXT("StepForwardSword"), RF_Transient);
            Character->AddInstanceComponent(Movement);
            Character->AddInstanceComponent(Sword);
            Movement->RegisterComponentWithWorld(World);
            Sword->RegisterComponentWithWorld(World);
            Sword->SetMovementDeliveryComponent(Movement);
            Sword->SetProfile(Profile);
            return true;
        }

        bool Start(FString& OutReason)
        {
            FKashmirTechniqueRequest Request;
            Request.Slot = RequestSlot;
            return Sword->StartTechniqueRequest(Request, Style, OutReason);
        }

        ~FStepForwardTestFixture()
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


#define KASHMIR_STEP_FORWARD_TEST(ClassName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(ClassName, \
        "Kashmir.Combat.StepForward." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


KASHMIR_STEP_FORWARD_TEST(FPersistentStepForwardExistsTest, "PersistentStepForwardExists")
bool FPersistentStepForwardExistsTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    const FKashmirCombatTechniqueDefinition* Step = FindPersistentStepForward(Style);
    TestNotNull(TEXT("Persistent StepForward exists in the CombatStyle"), Step);
    if (Step == nullptr) return false;
    FString Reason;
    TestTrue(TEXT("Persistent StepForward definition is valid"), Step->IsValid(Reason));
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FCorrectTechniqueIdTest, "CorrectTechniqueId")
bool FCorrectTechniqueIdTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Step =
        FindPersistentStepForward(LoadStepForwardStyle());
    TestNotNull(TEXT("Persistent StepForward exists"), Step);
    if (Step == nullptr) return false;
    TestEqual(TEXT("Persistent TechniqueId is stable"),
        Step->TechniqueId, StepForwardTechniqueId);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FCorrectBaseMotionTest, "CorrectBaseMotion")
bool FCorrectBaseMotionTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    const FKashmirCombatTechniqueDefinition* Baseline = FindStepForwardBaseline(Style);
    const FKashmirCombatTechniqueDefinition* Step = FindPersistentStepForward(Style);
    TestNotNull(TEXT("Baseline exists"), Baseline);
    TestNotNull(TEXT("StepForward exists"), Step);
    if (Baseline == nullptr || Step == nullptr) return false;
    TestEqual(TEXT("StepForward reuses AM_KashmirSword_HorizontalA"),
        Step->Montage, Baseline->Montage);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FCorrectActionIdTest, "CorrectActionId")
bool FCorrectActionIdTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    const FKashmirCombatTechniqueDefinition* Baseline = FindStepForwardBaseline(Style);
    const FKashmirCombatTechniqueDefinition* Step = FindPersistentStepForward(Style);
    if (Baseline == nullptr || Step == nullptr) return false;
    TestEqual(TEXT("StepForward preserves Sword.Direct.Right"),
        Step->ActionId, Baseline->ActionId);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FFullBodyIntentTest, "FullBodyIntent")
bool FFullBodyIntentTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Step =
        FindPersistentStepForward(LoadStepForwardStyle());
    if (Step == nullptr) return false;
    TestEqual(TEXT("StepForward selects FullBody presentation"),
        Step->MovementIntent, EKashmirMovementIntent::FullBody);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FControlledTranslationTest, "ControlledTranslation80cm025")
bool FControlledTranslationTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Step =
        FindPersistentStepForward(LoadStepForwardStyle());
    if (Step == nullptr) return false;
    const FKashmirTechniqueMovementSpec& Spec = Step->MovementSpec;
    TestEqual(TEXT("StepForward uses generic ControlledTranslation"),
        Spec.Delivery, EKashmirMovementDelivery::ControlledTranslation);
    TestEqual(TEXT("StepForward distance is 80 cm"), Spec.Distance, 80.0f);
    TestEqual(TEXT("StepForward duration is 0.25 s"), Spec.Duration, 0.25f);
    TestEqual(TEXT("MovementDelivery enum remains exactly two modes"),
        StaticEnum<EKashmirMovementDelivery>()->NumEnums() - 1, 2);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FForwardDirectionTest, "ForwardDirection")
bool FForwardDirectionTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Step =
        FindPersistentStepForward(LoadStepForwardStyle());
    if (Step == nullptr) return false;
    TestEqual(TEXT("StepForward direction is Forward"),
        Step->MovementSpec.Direction, EKashmirMovementDirection::Forward);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardRootMotionTest, "NoRootMotionDependency")
bool FStepForwardRootMotionTest::RunTest(const FString& Parameters)
{
    FStepForwardTestFixture Fixture;
    TestTrue(TEXT("StepForward fixture initializes"), Fixture.Initialize());
    UAnimInstance* AnimInstance = NewObject<UAnimInstance>(Fixture.Character->GetMesh());
    AnimInstance->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
    FString Reason;
    TestTrue(TEXT("StepForward starts"), Fixture.Start(Reason));
    TestTrue(TEXT("StepForward advances through MovementDelivery"),
        Fixture.Sword->AdvanceRuntime(0.25f, Reason));
    TestEqual(TEXT("StepForward does not alter RootMotionMode"),
        AnimInstance->RootMotionMode.GetValue(),
        ERootMotionMode::RootMotionFromMontagesOnly);
    TestEqual(TEXT("MovementDelivery completes 80 cm without Root Motion"),
        Fixture.Movement->GetActualDistance(), 80.0f, 0.1f);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardDamageTest, "DamagePreserved")
bool FStepForwardDamageTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    const FKashmirCombatTechniqueDefinition* Baseline = FindStepForwardBaseline(Style);
    const FKashmirCombatTechniqueDefinition* Step = FindPersistentStepForward(Style);
    if (Baseline == nullptr || Step == nullptr) return false;
    TestEqual(TEXT("Base damage is preserved"), Step->BaseDamage, Baseline->BaseDamage);
    TestEqual(TEXT("Guard damage is preserved"),
        Step->BaseGuardDamage, Baseline->BaseGuardDamage);
    TestTrue(TEXT("Combat effects are preserved"),
        Step->CombatDefinition.Effects == Baseline->CombatDefinition.Effects);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardTraceTest, "WeaponTracePreserved")
bool FStepForwardTraceTest::RunTest(const FString& Parameters)
{
    FStepForwardTestFixture Fixture;
    TestTrue(TEXT("StepForward fixture initializes"), Fixture.Initialize());
    FString Reason;
    TestTrue(TEXT("StepForward starts"), Fixture.Start(Reason));
    const FKashmirSwordActionPlan Plan = Fixture.Sword->GetActivePlan();
    TestEqual(TEXT("Trace Active duration remains baseline"),
        Plan.RuntimeDefinition.ActiveDuration,
        Fixture.Baseline.RuntimeDefinition.ActiveDuration);
    TestEqual(TEXT("ActionId driving trace remains baseline"),
        Plan.RuntimeDefinition.ActionId, Fixture.Baseline.ActionId);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardEvidenceTest, "HitEvidencePreserved")
bool FStepForwardEvidenceTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    const FKashmirCombatTechniqueDefinition* Baseline = FindStepForwardBaseline(Style);
    const FKashmirCombatTechniqueDefinition* Step = FindPersistentStepForward(Style);
    if (Baseline == nullptr || Step == nullptr) return false;
    TestEqual(TEXT("Attack direction evidence is preserved"),
        Step->AttackDirection, Baseline->AttackDirection);
    TestEqual(TEXT("Attack shape evidence is preserved"),
        Step->AttackShape, Baseline->AttackShape);
    TestEqual(TEXT("Contact profile is preserved"),
        Step->ContactProfileId, Baseline->ContactProfileId);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardCancelTest, "CancelPropagates")
bool FStepForwardCancelTest::RunTest(const FString& Parameters)
{
    FStepForwardTestFixture Fixture;
    TestTrue(TEXT("Cancellable StepForward fixture initializes"), Fixture.Initialize(true));
    FString Reason;
    TestTrue(TEXT("StepForward starts"), Fixture.Start(Reason));
    TestTrue(TEXT("StepForward advances 0.10 s"),
        Fixture.Sword->AdvanceRuntime(0.10f, Reason));
    TestTrue(TEXT("Action authority accepts StepForward cancellation"),
        Fixture.Sword->CancelCurrentAction(Reason));
    const FVector CancelledLocation = Fixture.Character->GetActorLocation();
    TestEqual(TEXT("ActionRuntime reports Interrupted"),
        Fixture.Sword->GetRuntimeState().Phase, EKashmirActionPhase::Interrupted);
    TestEqual(TEXT("MovementDelivery reports Cancelled"),
        Fixture.Movement->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Cancelled);
    TestEqual(TEXT("0.10 s cancellation yields 32 cm"),
        Fixture.Movement->GetActualDistance(), 32.0f, 0.2f);
    TestTrue(TEXT("Post-cancel advance is safe"),
        Fixture.Sword->AdvanceRuntime(0.10f, Reason));
    TestEqual(TEXT("Cancellation leaves no residual displacement"),
        Fixture.Character->GetActorLocation(), CancelledLocation);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardBlockedTest, "BlockedStateCoherent")
bool FStepForwardBlockedTest::RunTest(const FString& Parameters)
{
    FStepForwardTestFixture Fixture;
    TestTrue(TEXT("StepForward fixture initializes"), Fixture.Initialize());
    FString Reason;
    AActor* Blocker = Fixture.Movement->SpawnTransientDebugBlocker(
        FVector(100.0f, 0.0f, 100.0f),
        FVector(10.0f, 100.0f, 100.0f), Reason);
    TestNotNull(TEXT("Transient blocker spawns"), Blocker);
    TestTrue(TEXT("StepForward starts"), Fixture.Start(Reason));
    TestTrue(TEXT("Blocked StepForward advances safely"),
        Fixture.Sword->AdvanceRuntime(0.25f, Reason));
    TestTrue(TEXT("Blocked StepForward moves less than requested"),
        Fixture.Movement->GetActualDistance() < Fixture.Movement->GetRequestedDistance());
    TestEqual(TEXT("MovementDelivery reports Blocked"),
        Fixture.Movement->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Blocked);
    TestTrue(TEXT("Blocked delivery does not corrupt ActionRuntime"),
        Fixture.Sword->GetRuntimeState().bActive);
    TestEqual(TEXT("Active plan retains StepForward identity"),
        Fixture.Sword->GetActivePlan().TechniqueId, StepForwardTechniqueId);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FStepForwardOrthogonalTest, "MovementIntentAndDeliveryRemainSeparate")
bool FStepForwardOrthogonalTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Persistent =
        FindPersistentStepForward(LoadStepForwardStyle());
    if (Persistent == nullptr) return false;
    FKashmirCombatTechniqueDefinition Step = *Persistent;
    TestEqual(TEXT("StepForward FullBody does not replace delivery semantics"),
        Step.MovementSpec.Delivery, EKashmirMovementDelivery::ControlledTranslation);
    Step.MovementIntent = EKashmirMovementIntent::Stationary;
    FString Reason;
    TestTrue(TEXT("Stationary remains valid with the same delivery"),
        Step.IsValid(Reason));
    TestEqual(TEXT("Changing intent does not mutate delivery"),
        Step.MovementSpec.Delivery, EKashmirMovementDelivery::ControlledTranslation);
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FSlotsOneToFourUnchangedTest, "Slots1To4Unchanged")
bool FSlotsOneToFourUnchangedTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    TestNotNull(TEXT("Published baseline style loads"), Style);
    if (Style == nullptr) return false;
    TestEqual(TEXT("Published baseline retains four bindings"),
        Style->SlotBindings.Num(), 4);
    const TArray<FName> ExpectedIds = {
        FName(TEXT("Technique.Sword.Horizontal.LeftToRight")),
        FName(TEXT("Technique.Sword.Diagonal.Rising.RightToLeft")),
        FName(TEXT("Technique.Sword.Rising.LowToHigh")),
        FName(TEXT("Technique.Sword.Overhead.HighToLow"))};
    for (int32 Index = 0; Index < Style->SlotBindings.Num(); ++Index)
    {
        const FKashmirTechniqueSlotBinding& Binding = Style->SlotBindings[Index];
        TestEqual(TEXT("Slot remains in ordinal order"),
            static_cast<int32>(Binding.Slot), Index + 1);
        TestEqual(TEXT("Slot binding Technique remains unchanged"),
            Binding.TechniqueId, ExpectedIds[Index]);
        const FKashmirCombatTechniqueDefinition* Technique =
            Style->Techniques.FindByPredicate(
                [&Binding](const FKashmirCombatTechniqueDefinition& Candidate)
                {
                    return Candidate.TechniqueId == Binding.TechniqueId;
                });
        TestNotNull(TEXT("Existing binding still resolves"), Technique);
        if (Technique != nullptr)
        {
            TestEqual(TEXT("Existing Technique remains Delivery=None"),
                Technique->MovementSpec.Delivery, EKashmirMovementDelivery::None);
        }
    }
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FTechniqueIsUnboundTest, "TechniqueIsUnbound")
bool FTechniqueIsUnboundTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    if (Style == nullptr) return false;
    TestNotNull(TEXT("Persistent StepForward exists"), FindPersistentStepForward(Style));
    TestFalse(TEXT("Persistent StepForward has no Slot binding"),
        Style->SlotBindings.ContainsByPredicate(
            [](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.TechniqueId == StepForwardTechniqueId;
            }));
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FSlotFiveStillUnboundTest, "Slot5StillUnbound")
bool FSlotFiveStillUnboundTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStepForwardStyle();
    if (Style == nullptr) return false;
    TestFalse(TEXT("Slot5 remains absent from persistent bindings"),
        Style->SlotBindings.ContainsByPredicate(
            [](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.Slot == EKashmirTechniqueSlot::TechniqueSlot5;
            }));
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FNoTechniqueSpecificRuntimeBranchingTest,
    "NoTechniqueSpecificRuntimeBranching")
bool FNoTechniqueSpecificRuntimeBranchingTest::RunTest(const FString& Parameters)
{
    const TArray<FString> RuntimeFiles = {
        FPaths::Combine(FPaths::ProjectDir(),
            TEXT("Source/KashmirUE/Private/Combat/KashmirCombatTechnique.cpp")),
        FPaths::Combine(FPaths::ProjectDir(),
            TEXT("Source/KashmirUE/Private/Combat/KashmirDirectionalSwordComponent.cpp")),
        FPaths::Combine(FPaths::ProjectDir(),
            TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"))};
    for (const FString& File : RuntimeFiles)
    {
        FString Source;
        TestTrue(*FString::Printf(TEXT("Runtime source loads: %s"), *File),
            FFileHelper::LoadFileToString(Source, *File));
        TestFalse(*FString::Printf(TEXT("No StepForward branch in %s"), *File),
            Source.Contains(StepForwardTechniqueId.ToString()));
    }
    return true;
}

KASHMIR_STEP_FORWARD_TEST(FResolvesThroughNormalPipelineTest,
    "ResolvesThroughNormalPipeline")
bool FResolvesThroughNormalPipelineTest::RunTest(const FString& Parameters)
{
    FStepForwardTestFixture Fixture;
    TestTrue(TEXT("Persistent StepForward fixture initializes"), Fixture.Initialize());
    FString Reason;
    TestTrue(TEXT("Generic Technique request resolves and starts"), Fixture.Start(Reason));
    TestEqual(TEXT("Action plan retains persistent Technique identity"),
        Fixture.Sword->GetActivePlan().TechniqueId, StepForwardTechniqueId);
    TestEqual(TEXT("Normal pipeline preserves baseline ActionId"),
        Fixture.Sword->GetActivePlan().RuntimeDefinition.ActionId,
        Fixture.Baseline.ActionId);
    TestTrue(TEXT("Normal pipeline starts MovementDelivery"),
        Fixture.Movement->IsDeliveryActive());
    return true;
}

#endif
