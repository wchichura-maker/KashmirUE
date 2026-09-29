#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimMontage.h"
#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirCombatantComponent.h"
#include "Combat/KashmirDirectionalMeleeResolver.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirHitRegionMap.h"
#include "Combat/KashmirWeaponTraceComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EnhancedActionKeyMapping.h"
#include "GameFramework/Actor.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "KashmirCharacter.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"


namespace
{
    constexpr const TCHAR* SwordStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* SwordProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");

    struct FExpectedSwordSlot
    {
        EKashmirTechniqueSlot Slot;
        const TCHAR* TechniqueId;
        const TCHAR* ActionId;
        const TCHAR* TechniqueFamily;
        const TCHAR* MontagePath;
        EKashmirAttackDirection Direction;
        EKashmirAttackShape Shape;
    };

    const TArray<FExpectedSwordSlot>& ExpectedSwordSlots()
    {
        static const TArray<FExpectedSwordSlot> Expected = {
            {EKashmirTechniqueSlot::TechniqueSlot1,
                TEXT("Technique.Sword.Horizontal.LeftToRight"),
                TEXT("Sword.Direct.Right"),
                TEXT("Horizontal"),
                TEXT("/Game/KashmirAct/Combat/DirectionalSword/Montages/AM_KashmirSword_HorizontalA.AM_KashmirSword_HorizontalA"),
                EKashmirAttackDirection::LeftToRight,
                EKashmirAttackShape::Slash},
            {EKashmirTechniqueSlot::TechniqueSlot2,
                TEXT("Technique.Sword.Diagonal.Rising.RightToLeft"),
                TEXT("Sword.Direct.Left"),
                TEXT("DiagonalRising"),
                TEXT("/Game/KashmirAct/Combat/DirectionalSword/Montages/AM_KashmirSword_HorizontalB.AM_KashmirSword_HorizontalB"),
                EKashmirAttackDirection::RightToLeft,
                EKashmirAttackShape::Slash},
            {EKashmirTechniqueSlot::TechniqueSlot3,
                TEXT("Technique.Sword.Rising.LowToHigh"),
                TEXT("Sword.Direct.Up"),
                TEXT("Rising"),
                TEXT("/Game/KashmirAct/Combat/DirectionalSword/Montages/AM_KashmirSword_VerticalA.AM_KashmirSword_VerticalA"),
                EKashmirAttackDirection::LowToHigh,
                EKashmirAttackShape::Slash},
            {EKashmirTechniqueSlot::TechniqueSlot4,
                TEXT("Technique.Sword.Overhead.HighToLow"),
                TEXT("Sword.Direct.Down"),
                TEXT("Overhead"),
                TEXT("/Game/KashmirAct/Combat/DirectionalSword/Montages/AM_KashmirSword_VerticalB.AM_KashmirSword_VerticalB"),
                EKashmirAttackDirection::HighToLow,
                EKashmirAttackShape::Slash}
        };
        return Expected;
    }

    UKashmirWeaponCombatStyle* LoadSwordStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(nullptr, SwordStylePath);
    }

    UKashmirDirectionalSwordProfile* LoadSwordProfile()
    {
        return LoadObject<UKashmirDirectionalSwordProfile>(nullptr, SwordProfilePath);
    }

    const FKashmirCombatTechniqueDefinition* FindTechnique(
        const UKashmirWeaponCombatStyle* Style,
        const FName TechniqueId)
    {
        return Style != nullptr
            ? Style->Techniques.FindByPredicate(
                [TechniqueId](const FKashmirCombatTechniqueDefinition& Candidate)
                {
                    return Candidate.TechniqueId == TechniqueId;
                })
            : nullptr;
    }

    struct FPlayableSwordTestWorld
    {
        UWorld* World = nullptr;
        AActor* WeaponActor = nullptr;
        USceneComponent* TraceSource = nullptr;
        UKashmirWeaponTraceComponent* Trace = nullptr;
        UKashmirDirectionalSwordComponent* Sword = nullptr;
        AActor* TargetActor = nullptr;

        bool Initialize()
        {
            const FName WorldName = MakeUniqueObjectName(
                nullptr,
                UWorld::StaticClass(),
                TEXT("KashmirPlayableSwordGrammarWorld"),
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

            WeaponActor = World->SpawnActor<AActor>(
                FVector::ZeroVector, FRotator::ZeroRotator);
            if (WeaponActor == nullptr)
            {
                return false;
            }
            TraceSource = NewObject<USceneComponent>(
                WeaponActor, TEXT("TechniqueTraceSource"), RF_Transient);
            WeaponActor->SetRootComponent(TraceSource);
            WeaponActor->AddInstanceComponent(TraceSource);
            TraceSource->RegisterComponentWithWorld(World);

            Trace = NewObject<UKashmirWeaponTraceComponent>(
                WeaponActor, TEXT("TechniqueWeaponTrace"), RF_Transient);
            WeaponActor->AddInstanceComponent(Trace);
            Trace->RegisterComponentWithWorld(World);
            Trace->SetTraceSource(TraceSource);
            Trace->SetIgnoredActor(WeaponActor);
            FKashmirWeaponContactPointBinding TraceBinding;
            TraceBinding.Id = TEXT("Weapon_Tip");
            TraceBinding.Component = TraceSource;
            Trace->SetContactPointBindings({TraceBinding});

            Sword = NewObject<UKashmirDirectionalSwordComponent>(
                WeaponActor, TEXT("TechniqueSword"), RF_Transient);
            WeaponActor->AddInstanceComponent(Sword);
            Sword->RegisterComponentWithWorld(World);
            Sword->SetProfile(LoadSwordProfile());
            Sword->SetWeaponTraceComponent(Trace);

            TargetActor = World->SpawnActor<AActor>(
                FVector(50.0, 0.0, 0.0), FRotator::ZeroRotator);
            if (TargetActor == nullptr)
            {
                return false;
            }
            UBoxComponent* TargetBox = NewObject<UBoxComponent>(
                TargetActor, TEXT("TechniqueTargetBox"), RF_Transient);
            TargetActor->SetRootComponent(TargetBox);
            TargetActor->AddInstanceComponent(TargetBox);
            TargetBox->SetBoxExtent(FVector(10.0, 20.0, 20.0));
            TargetBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            TargetBox->SetCollisionObjectType(ECC_Pawn);
            TargetBox->SetCollisionResponseToAllChannels(ECR_Ignore);
            TargetBox->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
            TargetBox->RegisterComponentWithWorld(World);
            TargetBox->UpdateComponentToWorld();
            World->Tick(LEVELTICK_All, 0.0f);
            return true;
        }

        ~FPlayableSwordTestWorld()
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


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueSlotInputContractTest,
    "Kashmir.Combat.PlayableGrammar.TechniqueSlotInputContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTechniqueSlotInputContractTest::RunTest(const FString& Parameters)
{
    const UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Input/IMC_Player.IMC_Player"));
    TestNotNull(TEXT("Player mapping context loads"), MappingContext);
    if (MappingContext == nullptr)
    {
        return false;
    }

    const TArray<FKey> ExpectedKeys = {
        EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five};
    TSet<const UInputAction*> TechniqueActions;
    for (int32 Index = 0; Index < ExpectedKeys.Num(); ++Index)
    {
        const FString Name = FString::Printf(TEXT("IA_TechniqueSlot%d"), Index + 1);
        const FString Path = FString::Printf(
            TEXT("/Game/KashmirAct/Characters/Player/Input/%s.%s"), *Name, *Name);
        const UInputAction* Action = LoadObject<UInputAction>(nullptr, *Path);
        TestNotNull(*FString::Printf(TEXT("%s exists"), *Name), Action);
        if (Action == nullptr)
        {
            continue;
        }
        TestEqual(*FString::Printf(TEXT("%s is boolean"), *Name),
            Action->ValueType, EInputActionValueType::Boolean);
        TechniqueActions.Add(Action);
        TestTrue(*FString::Printf(TEXT("%s maps to baseline key"), *Name),
            MappingContext->GetMappings().ContainsByPredicate(
                [Action, &ExpectedKeys, Index](const FEnhancedActionKeyMapping& Mapping)
                {
                    return Mapping.Action == Action && Mapping.Key == ExpectedKeys[Index];
                }));
    }
    TestEqual(TEXT("Five distinct logical technique input actions exist"),
        TechniqueActions.Num(), 5);

    const UInputAction* Gesture = LoadObject<UInputAction>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Input/")
        TEXT("IA_SwordGestureHold.IA_SwordGestureHold"));
    TestFalse(TEXT("No MMB gesture mapping is active"),
        MappingContext->GetMappings().ContainsByPredicate(
            [Gesture](const FEnhancedActionKeyMapping& Mapping)
            {
                return Mapping.Action == Gesture || Mapping.Key == EKeys::MiddleMouseButton;
            }));

    UClass* CharacterClass = LoadObject<UClass>(nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Blueprints/")
        TEXT("BP_KashmirCharacter.BP_KashmirCharacter_C"));
    TestNotNull(TEXT("Player character Blueprint loads"), CharacterClass);
    if (CharacterClass != nullptr)
    {
        const UObject* Defaults = CharacterClass->GetDefaultObject();
        for (int32 Index = 0; Index < 5; ++Index)
        {
            const FName PropertyName(*FString::Printf(
                TEXT("TechniqueSlot%dAction"), Index + 1));
            const FObjectPropertyBase* Property =
                FindFProperty<FObjectPropertyBase>(CharacterClass, PropertyName);
            TestNotNull(*FString::Printf(TEXT("%s property exists"),
                *PropertyName.ToString()), Property);
            if (Property != nullptr)
            {
                TestNotNull(*FString::Printf(TEXT("%s is assigned"),
                    *PropertyName.ToString()),
                    Property->GetObjectPropertyValue_InContainer(Defaults));
            }
        }
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordStyleSlotResolutionTest,
    "Kashmir.Combat.PlayableGrammar.SwordStyleSlotResolution",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirSwordStyleSlotResolutionTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    TestNotNull(TEXT("Sword combat style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    FString Reason;
    TestTrue(TEXT("Sword combat style validates"), Style->ValidateStyle(Reason));
    TestEqual(TEXT("Baseline has four perceptually distinct slot bindings"),
        Style->SlotBindings.Num(), 4);
    for (const FExpectedSwordSlot& Expected : ExpectedSwordSlots())
    {
        FKashmirTechniqueRequest Request;
        Request.Slot = Expected.Slot;
        FKashmirTechniqueActionPlan Plan;
        TestTrue(*FString::Printf(TEXT("Slot resolves %s"), Expected.TechniqueId),
            Style->ResolveTechnique(Request, Plan, Reason));
        TestEqual(TEXT("Technique id matches baseline"),
            Plan.Technique.TechniqueId, FName(Expected.TechniqueId));
        TestEqual(TEXT("Action id matches authored profile"),
            Plan.ActionRequest.ActionId, FName(Expected.ActionId));
        TestEqual(TEXT("Direction metadata is preserved"),
            Plan.Technique.AttackDirection, Expected.Direction);
        TestFalse(TEXT("Technique has presentation montage"),
            Plan.Technique.Montage.IsNull());
        TestTrue(TEXT("Technique retains contact damage effect"),
            Plan.Technique.CombatDefinition.Effects.Contains(
                EKashmirResolutionType::Damage));
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirUnboundTechniqueSlotTest,
    "Kashmir.Combat.PlayableGrammar.UnboundTechniqueSlot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirUnboundTechniqueSlotTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    UKashmirDirectionalSwordProfile* Profile = LoadSwordProfile();
    TestNotNull(TEXT("Sword style loads"), Style);
    TestNotNull(TEXT("Sword profile loads"), Profile);
    if (Style == nullptr || Profile == nullptr)
    {
        return false;
    }

    FKashmirTechniqueRequest Request;
    Request.Slot = EKashmirTechniqueSlot::TechniqueSlot5;
    FKashmirTechniqueActionPlan Plan;
    FString Reason;
    TestFalse(TEXT("Reserved Slot5 does not resolve an action plan"),
        Style->ResolveTechnique(Request, Plan, Reason));
    TestFalse(TEXT("Rejected Slot5 leaves no resolved plan"), Plan.bResolved);
    TestTrue(TEXT("Rejected Slot5 provides a useful reason"), !Reason.IsEmpty());

    UKashmirDirectionalSwordComponent* Component =
        NewObject<UKashmirDirectionalSwordComponent>();
    Component->SetProfile(Profile);
    TestFalse(TEXT("Reserved Slot5 safely rejects runtime start"),
        Component->StartTechniqueRequest(Request, Style, Reason));
    TestFalse(TEXT("Reserved Slot5 does not activate ActionRuntime"),
        Component->GetRuntimeState().bActive);
    TestTrue(TEXT("Reserved Slot5 does not select a random montage"),
        Component->GetActivePlan().Montage.IsNull());
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordTechniqueRuntimeBridgeTest,
    "Kashmir.Combat.PlayableGrammar.SwordTechniqueRuntimeBridge",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirSwordTechniqueRuntimeBridgeTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    UKashmirDirectionalSwordProfile* Profile = LoadSwordProfile();
    TestNotNull(TEXT("Sword style loads"), Style);
    TestNotNull(TEXT("Sword profile loads"), Profile);
    if (Style == nullptr || Profile == nullptr)
    {
        return false;
    }
    for (const FExpectedSwordSlot& Expected : ExpectedSwordSlots())
    {
        UKashmirDirectionalSwordComponent* Component =
            NewObject<UKashmirDirectionalSwordComponent>();
        Component->SetProfile(Profile);
        FKashmirTechniqueRequest Request;
        Request.Slot = Expected.Slot;
        FString Reason;
        TestTrue(*FString::Printf(TEXT("Runtime starts %s"), Expected.ActionId),
            Component->StartTechniqueRequest(Request, Style, Reason));
        TestEqual(TEXT("Runtime action matches slot resolution"),
            Component->GetRuntimeState().ActionId, FName(Expected.ActionId));
        TestEqual(TEXT("Presentation montage matches technique"),
            Component->GetActivePlan().Montage,
            FindTechnique(Style, FName(Expected.TechniqueId))->Montage);
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordTechniqueDirectionMetadataTest,
    "Kashmir.Combat.PlayableGrammar.SwordTechniqueDirectionMetadata",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirSwordTechniqueDirectionMetadataTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    for (const FExpectedSwordSlot& Expected : ExpectedSwordSlots())
    {
        const FKashmirCombatTechniqueDefinition* Technique =
            FindTechnique(Style, FName(Expected.TechniqueId));
        TestNotNull(TEXT("Expected technique exists"), Technique);
        if (Technique != nullptr)
        {
            TestEqual(TEXT("Authored direction metadata matches baseline"),
                Technique->AttackDirection, Expected.Direction);
            TestEqual(TEXT("Kinematic family matches the inspected base motion"),
                Technique->TechniqueFamily, FName(Expected.TechniqueFamily));
            TestEqual(TEXT("Broad attack shape remains semantically compatible"),
                Technique->AttackShape, Expected.Shape);
            TestEqual(TEXT("Inspected base montage remains bound to the technique"),
                Technique->Montage.ToSoftObjectPath().GetAssetPathString(),
                FString(Expected.MontagePath));
            TestFalse(TEXT("Authored direction produces a non-zero vector"),
                Technique->GetAuthoredDirectionVector().IsNearlyZero());
        }
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordTechniquePresentationTest,
    "Kashmir.Combat.PlayableGrammar.SwordTechniquePresentation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirSwordTechniquePresentationTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    for (const FKashmirCombatTechniqueDefinition& Technique : Style->Techniques)
    {
        TestFalse(TEXT("Technique has an authored montage"),
            Technique.Montage.IsNull());
        TestNotNull(TEXT("Technique montage loads"), Technique.Montage.LoadSynchronous());
        TestTrue(TEXT("Technique play rate is positive"), Technique.PlayRate > 0.0f);
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDistinctTechniquePresentationTest,
    "Kashmir.Combat.PlayableGrammar.DistinctTechniquePresentation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirDistinctTechniquePresentationTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }

    TSet<FSoftObjectPath> BaselinePresentations;
    for (const FKashmirTechniqueSlotBinding& Binding : Style->SlotBindings)
    {
        const FKashmirCombatTechniqueDefinition* Technique =
            FindTechnique(Style, Binding.TechniqueId);
        TestNotNull(TEXT("Bound technique exists"), Technique);
        if (Technique == nullptr)
        {
            continue;
        }
        const FSoftObjectPath Presentation = Technique->Montage.ToSoftObjectPath();
        TestFalse(TEXT("Distinct baseline technique has a unique presentation"),
            BaselinePresentations.Contains(Presentation));
        BaselinePresentations.Add(Presentation);
    }
    TestEqual(TEXT("Four bound techniques have four distinct montages"),
        BaselinePresentations.Num(), 4);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniquePresentationConsistencyTest,
    "Kashmir.Combat.PlayableGrammar.TechniquePresentationConsistency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTechniquePresentationConsistencyTest::RunTest(
    const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    for (const FExpectedSwordSlot& Expected : ExpectedSwordSlots())
    {
        FKashmirTechniqueRequest Request;
        Request.Slot = Expected.Slot;
        FKashmirTechniqueActionPlan Plan;
        FString Reason;
        TestTrue(TEXT("Bound technique resolves for consistency check"),
            Style->ResolveTechnique(Request, Plan, Reason));
        TestEqual(TEXT("Resolved direction matches declared presentation baseline"),
            Plan.Technique.AttackDirection, Expected.Direction);
        TestFalse(TEXT("Resolved presentation exists"), Plan.Technique.Montage.IsNull());
        TestTrue(TEXT("Physical active window remains positive"),
            Plan.Technique.RuntimeDefinition.ActiveDuration > 0.0f);
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordTechniqueWeaponTraceCompatibilityTest,
    "Kashmir.Combat.PlayableGrammar.SwordTechniqueWeaponTraceCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirSwordTechniqueWeaponTraceCompatibilityTest::RunTest(
    const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    for (const FKashmirCombatTechniqueDefinition& Technique : Style->Techniques)
    {
        TestTrue(TEXT("Technique exposes a positive active trace window"),
            Technique.RuntimeDefinition.ActiveDuration > 0.0f);
        TestEqual(TEXT("Technique uses physical contact delivery"),
            Technique.CombatDefinition.Delivery, EKashmirDeliveryType::Contact);
        TestTrue(TEXT("Technique preserves physical damage resolution"),
            Technique.CombatDefinition.Effects.Contains(EKashmirResolutionType::Damage));
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirNoGestureInputDependencyTest,
    "Kashmir.Combat.PlayableGrammar.NoGestureInputDependency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirNoGestureInputDependencyTest::RunTest(const FString& Parameters)
{
    const UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Input/IMC_Player.IMC_Player"));
    const UInputAction* Gesture = LoadObject<UInputAction>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Input/")
        TEXT("IA_SwordGestureHold.IA_SwordGestureHold"));
    TestNotNull(TEXT("Player mapping context loads"), MappingContext);
    if (MappingContext == nullptr)
    {
        return false;
    }
    TestFalse(TEXT("Legacy gesture action is not actively mapped"),
        MappingContext->GetMappings().ContainsByPredicate(
            [Gesture](const FEnhancedActionKeyMapping& Mapping)
            {
                return Mapping.Action == Gesture;
            }));
    TestFalse(TEXT("Middle mouse is not an attack selection input"),
        MappingContext->GetMappings().ContainsByPredicate(
            [](const FEnhancedActionKeyMapping& Mapping)
            {
                return Mapping.Key == EKeys::MiddleMouseButton;
            }));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirPreserveNoGestureDependencyTest,
    "Kashmir.Combat.PlayableGrammar.PreserveNoGestureDependency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirPreserveNoGestureDependencyTest::RunTest(const FString& Parameters)
{
    const UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Input/IMC_Player.IMC_Player"));
    TestNotNull(TEXT("Player mapping context loads"), MappingContext);
    return MappingContext != nullptr && TestFalse(
        TEXT("Superseded MMB gesture remains outside the playable baseline"),
        MappingContext->GetMappings().ContainsByPredicate(
            [](const FEnhancedActionKeyMapping& Mapping)
            {
                return Mapping.Key == EKeys::MiddleMouseButton;
            }));
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueSlotRebindingBoundaryTest,
    "Kashmir.Combat.PlayableGrammar.TechniqueSlotRebindingBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTechniqueSlotRebindingBoundaryTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Source = LoadSwordStyle();
    TestNotNull(TEXT("Sword style loads"), Source);
    if (Source == nullptr)
    {
        return false;
    }
    UKashmirWeaponCombatStyle* Rebound = DuplicateObject<UKashmirWeaponCombatStyle>(
        Source, GetTransientPackage());
    TestNotNull(TEXT("Style duplicates for rebind test"), Rebound);
    if (Rebound == nullptr)
    {
        return false;
    }
    Swap(Rebound->SlotBindings[0].TechniqueId, Rebound->SlotBindings[1].TechniqueId);
    FKashmirTechniqueRequest Request;
    Request.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
    FKashmirTechniqueActionPlan Plan;
    FString Reason;
    TestTrue(TEXT("Rebound Slot1 resolves without core changes"),
        Rebound->ResolveTechnique(Request, Plan, Reason));
    TestEqual(TEXT("Slot1 follows style data, not hardcoded action"),
        Plan.ActionRequest.ActionId, FName(TEXT("Sword.Direct.Left")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirPlayableSwordPipelineTest,
    "Kashmir.Combat.PlayableGrammar.PhysicalSweepToCombatResult",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirPlayableSwordPipelineTest::RunTest(const FString& Parameters)
{
    FPlayableSwordTestWorld Fixture;
    TestTrue(TEXT("Playable sword test world initializes"), Fixture.Initialize());
    UKashmirWeaponCombatStyle* Style = LoadSwordStyle();
    if (Fixture.World == nullptr || Fixture.Sword == nullptr || Style == nullptr)
    {
        return false;
    }

    FKashmirTechniqueRequest Request;
    Request.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
    FString Reason;
    TestTrue(TEXT("Physical input-equivalent slot starts technique runtime"),
        Fixture.Sword->StartTechniqueRequest(Request, Style, Reason));
    const FKashmirSwordActionPlan Plan = Fixture.Sword->GetActivePlan();
    TestFalse(TEXT("Technique carries a montage"), Plan.Montage.IsNull());
    TestTrue(TEXT("Advance reaches authored active window"),
        Fixture.Sword->AdvanceRuntime(
            Plan.RuntimeDefinition.StartupDuration + 0.001f, Reason));
    TestTrue(TEXT("ActionRuntime opens real WeaponTrace window"),
        Fixture.Trace->IsTraceWindowActive());

    TArray<FKashmirWeaponTraceHit> Hits;
    TestTrue(TEXT("First trace sample initializes frame"),
        Fixture.Trace->SampleTrace(0.1f, Hits, Reason));
    Fixture.WeaponActor->SetActorLocation(FVector(100.0, 0.0, 0.0));
    Fixture.TraceSource->UpdateComponentToWorld();
    Fixture.World->Tick(LEVELTICK_All, 0.0f);
    TestTrue(TEXT("Active technique performs physical sweep"),
        Fixture.Trace->SampleTrace(0.1f, Hits, Reason));
    TestEqual(TEXT("Physical sweep hits target once"), Hits.Num(), 1);
    if (Hits.Num() != 1)
    {
        return false;
    }
    // The transient box has no skeleton, so provide the fixture's semantic
    // bone identity explicitly before exercising the production region map.
    Hits[0].Hit.BoneName = TEXT("test_target_body");

    FKashmirDirectionalSwordContact Contact;
    FKashmirDirectionalSwordContactResolver ContactResolver;
    TestTrue(TEXT("Sweep becomes resolved sword contact"),
        ContactResolver.Resolve(Hits[0], Plan, Contact, Reason));

    FKashmirDirectionalMeleeInput MeleeInput;
    MeleeInput.InstigatorId = TEXT("Player");
    MeleeInput.TargetId = TEXT("TargetDummy");
    MeleeInput.SourceId = Contact.SourceId;
    MeleeInput.HitRegionMap = NewObject<UKashmirHitRegionMap>();
    MeleeInput.HitRegionMap->BoneToRegion.Add(
        TEXT("test_target_body"),
        FGameplayTag::RequestGameplayTag(TEXT("HitRegion.Torso")));
    MeleeInput.DefenseInput.AvailableStamina = 100.0f;
    MeleeInput.DefenseInput.BlockState.ForwardDirection = FVector::ForwardVector;
    FKashmirDirectionalMeleeResult Result;
    FKashmirDirectionalMeleeResolver Resolver;
    const bool bMeleeResolved =
        Resolver.Resolve(Contact, MeleeInput, Result, Reason);
    if (!bMeleeResolved)
    {
        AddError(FString::Printf(
            TEXT("Physical sword contact failed melee resolution: %s"),
            *Reason));
    }
    TestTrue(TEXT("HitEvidence reaches defense/damage resolution"),
        bMeleeResolved);
    TestTrue(TEXT("CombatResult is delivered"),
        Result.Defense.HitResult.CombatResult.bDelivered);
    TestEqual(TEXT("CombatResult contains damage effect"),
        Result.Defense.HitResult.CombatResult.Effects.Num(), 1);

    UKashmirCombatantComponent* Target = NewObject<UKashmirCombatantComponent>();
    FKashmirCombatantApplicationResult Application;
    TestTrue(TEXT("Resolved result applies through persistent-state boundary"),
        Target->ApplyResolvedMelee(Result, Application, Reason));
    TestTrue(TEXT("Technique damage changes target health"),
        Target->GetHealthState().Current < Target->GetHealthState().Maximum);
    return true;
}

#endif
