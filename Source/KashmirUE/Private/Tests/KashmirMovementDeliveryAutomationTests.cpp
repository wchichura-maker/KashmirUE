#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirMovementDeliveryComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace
{
    constexpr const TCHAR* MovementDeliveryStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* MovementDeliveryProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");

    UKashmirWeaponCombatStyle* LoadMovementDeliveryStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(nullptr, MovementDeliveryStylePath);
    }

    UKashmirDirectionalSwordProfile* LoadMovementDeliveryProfile()
    {
        return LoadObject<UKashmirDirectionalSwordProfile>(nullptr, MovementDeliveryProfilePath);
    }

    struct FMovementDeliveryPlanWorld
    {
        UWorld* World = nullptr;
        ACharacter* Character = nullptr;

        bool Initialize()
        {
            const FName WorldName = MakeUniqueObjectName(
                nullptr, UWorld::StaticClass(), TEXT("KashmirMovementDeliveryPlanWorld"),
                EUniqueObjectNameOptions::GloballyUnique);
            FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
            World = UWorld::CreateWorld(
                EWorldType::Game, false, WorldName, GetTransientPackage());
            if (World == nullptr) return false;
            World->AddToRoot();
            Context.SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
            Character = World->SpawnActor<ACharacter>();
            return Character != nullptr;
        }

        ~FMovementDeliveryPlanWorld()
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
    FKashmirMovementDeliveryContractTest,
    "Kashmir.Combat.MovementDelivery.Contract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementDeliveryContractTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition Technique;
    const FKashmirSwordActionPlan Plan;
    TestEqual(TEXT("Technique defaults to no movement delivery"),
        Technique.MovementSpec.Delivery, EKashmirMovementDelivery::None);
    TestEqual(TEXT("Action plan defaults to no movement delivery"),
        Plan.MovementSpec.Delivery, EKashmirMovementDelivery::None);
    TestEqual(TEXT("Default distance is zero"), Technique.MovementSpec.Distance, 0.0f);
    TestEqual(TEXT("Default duration is zero"), Technique.MovementSpec.Duration, 0.0f);
    TestNotNull(TEXT("MovementIntent remains a distinct reflected property"),
        FindFProperty<FProperty>(FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementIntent")));
    TestNotNull(TEXT("MovementSpec is a distinct reflected property"),
        FindFProperty<FProperty>(FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementSpec")));
    TestNotEqual(TEXT("Movement intent and delivery are distinct enum types"),
        StaticEnum<EKashmirMovementIntent>()->GetFName(),
        StaticEnum<EKashmirMovementDelivery>()->GetFName());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementDeliveryBaselineTest,
    "Kashmir.Combat.MovementDelivery.Baseline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementDeliveryBaselineTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadMovementDeliveryStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr) return false;
    for (const FKashmirCombatTechniqueDefinition& Technique : Style->Techniques)
    {
        TestEqual(*FString::Printf(TEXT("%s keeps Delivery=None"),
            *Technique.TechniqueId.ToString()),
            Technique.MovementSpec.Delivery, EKashmirMovementDelivery::None);
        TestEqual(*FString::Printf(TEXT("%s keeps zero movement distance"),
            *Technique.TechniqueId.ToString()), Technique.MovementSpec.Distance, 0.0f);
        TestEqual(*FString::Printf(TEXT("%s keeps zero movement duration"),
            *Technique.TechniqueId.ToString()), Technique.MovementSpec.Duration, 0.0f);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementDeliveryIndependenceTest,
    "Kashmir.Combat.MovementDelivery.Independence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementDeliveryIndependenceTest::RunTest(const FString& Parameters)
{
    FKashmirCombatTechniqueDefinition StationaryTechnique;
    StationaryTechnique.MovementIntent = EKashmirMovementIntent::Stationary;
    StationaryTechnique.MovementSpec.Delivery =
        EKashmirMovementDelivery::ControlledTranslation;
    StationaryTechnique.MovementSpec.Distance = 80.0f;
    StationaryTechnique.MovementSpec.Duration = 0.25f;
    FString Reason;
    TestTrue(TEXT("Stationary presentation permits a valid controlled movement contract"),
        StationaryTechnique.MovementSpec.IsValid(Reason));
    TestEqual(TEXT("Controlled delivery does not change Stationary intent"),
        StationaryTechnique.MovementIntent, EKashmirMovementIntent::Stationary);

    FKashmirCombatTechniqueDefinition FullBodyTechnique;
    FullBodyTechnique.MovementIntent = EKashmirMovementIntent::FullBody;
    TestEqual(TEXT("FullBody does not imply controlled translation"),
        FullBodyTechnique.MovementSpec.Delivery, EKashmirMovementDelivery::None);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementDeliveryPlanTest,
    "Kashmir.Combat.MovementDelivery.PlanPropagation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementDeliveryPlanTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* SourceStyle = LoadMovementDeliveryStyle();
    UKashmirDirectionalSwordProfile* Profile = LoadMovementDeliveryProfile();
    TestNotNull(TEXT("Sword style loads"), SourceStyle);
    TestNotNull(TEXT("Sword profile loads"), Profile);
    if (SourceStyle == nullptr || Profile == nullptr ||
        SourceStyle->Techniques.IsEmpty() || SourceStyle->SlotBindings.IsEmpty())
    {
        return false;
    }

    UKashmirWeaponCombatStyle* Style = DuplicateObject<UKashmirWeaponCombatStyle>(
        SourceStyle, GetTransientPackage());
    FKashmirCombatTechniqueDefinition& Technique = Style->Techniques[0];
    Style->SlotBindings[0].TechniqueId = Technique.TechniqueId;
    const float OriginalDamage = Technique.BaseDamage;
    const float OriginalGuardDamage = Technique.BaseGuardDamage;
    const float OriginalActiveDuration = Technique.RuntimeDefinition.ActiveDuration;
    const FKashmirCombatActionDefinition OriginalCombatDefinition =
        Technique.CombatDefinition;
    Technique.MovementIntent = EKashmirMovementIntent::Stationary;
    Technique.MovementSpec.Delivery =
        EKashmirMovementDelivery::ControlledTranslation;
    Technique.MovementSpec.Distance = 80.0f;
    Technique.MovementSpec.Duration = 0.25f;
    Technique.MovementSpec.Direction = EKashmirMovementDirection::Forward;

    FMovementDeliveryPlanWorld Fixture;
    TestTrue(TEXT("Runtime world initializes"), Fixture.Initialize());
    if (Fixture.Character == nullptr) return false;
    UKashmirDirectionalSwordComponent* Sword =
        NewObject<UKashmirDirectionalSwordComponent>(Fixture.Character);
    UKashmirMovementDeliveryComponent* MovementDelivery =
        NewObject<UKashmirMovementDeliveryComponent>(Fixture.Character);
    Fixture.Character->AddInstanceComponent(Sword);
    Fixture.Character->AddInstanceComponent(MovementDelivery);
    Sword->RegisterComponentWithWorld(Fixture.World);
    MovementDelivery->RegisterComponentWithWorld(Fixture.World);
    Sword->SetMovementDeliveryComponent(MovementDelivery);
    Sword->SetProfile(Profile);
    FKashmirTechniqueRequest Request;
    Request.Slot = Style->SlotBindings[0].Slot;
    FString Reason;
    TestTrue(TEXT("Technique with movement contract resolves through existing runtime"),
        Sword->StartTechniqueRequest(Request, Style, Reason));
    const FKashmirSwordActionPlan Plan = Sword->GetActivePlan();
    TestEqual(TEXT("Movement delivery reaches the action plan"),
        Plan.MovementSpec.Delivery,
        EKashmirMovementDelivery::ControlledTranslation);
    TestEqual(TEXT("Movement distance reaches the action plan"),
        Plan.MovementSpec.Distance, 80.0f);
    TestEqual(TEXT("Movement duration reaches the action plan"),
        Plan.MovementSpec.Duration, 0.25f);
    TestEqual(TEXT("Movement direction reaches the action plan"),
        Plan.MovementSpec.Direction, EKashmirMovementDirection::Forward);
    TestEqual(TEXT("Movement contract preserves presentation intent"),
        Plan.MovementIntent, EKashmirMovementIntent::Stationary);
    TestEqual(TEXT("Movement contract preserves damage"),
        Plan.BaseDamage, OriginalDamage);
    TestEqual(TEXT("Movement contract preserves guard damage"),
        Plan.BaseGuardDamage, OriginalGuardDamage);
    TestEqual(TEXT("Movement contract preserves WeaponTrace active timing"),
        Plan.RuntimeDefinition.ActiveDuration, OriginalActiveDuration);
    TestEqual(TEXT("Movement contract preserves combat delivery"),
        Plan.CombatDefinition.Delivery, OriginalCombatDefinition.Delivery);
    TestEqual(TEXT("Movement contract preserves combat target"),
        Plan.CombatDefinition.Target, OriginalCombatDefinition.Target);
    TestTrue(TEXT("Movement contract preserves CombatResult effects"),
        Plan.CombatDefinition.Effects == OriginalCombatDefinition.Effects);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementDeliveryAuthorityBoundaryTest,
    "Kashmir.Combat.MovementDelivery.AuthorityBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementDeliveryAuthorityBoundaryTest::RunTest(const FString& Parameters)
{
    static const TCHAR* UnaffectedSources[] =
    {
        TEXT("Source/KashmirUE/Private/Combat/KashmirWeaponTraceComponent.cpp"),
        TEXT("Source/KashmirUE/Private/Combat/KashmirHitEvidenceBuilder.cpp"),
        TEXT("Source/KashmirUE/Private/Combat/KashmirHurtboxComponent.cpp"),
        TEXT("Source/KashmirUE/Private/Combat/KashmirDamageResolver.cpp"),
        TEXT("Source/KashmirUE/Private/Combat/KashmirCombatResolver.cpp")
    };
    for (const TCHAR* RelativePath : UnaffectedSources)
    {
        FString Source;
        const FString Path = FPaths::Combine(FPaths::ProjectDir(), RelativePath);
        TestTrue(*FString::Printf(TEXT("%s is readable"), RelativePath),
            FFileHelper::LoadFileToString(Source, *Path));
        TestFalse(*FString::Printf(TEXT("%s does not consume MovementDelivery"),
            RelativePath), Source.Contains(TEXT("MovementDelivery")));
        TestFalse(*FString::Printf(TEXT("%s does not consume MovementSpec"),
            RelativePath), Source.Contains(TEXT("MovementSpec")));
    }
    return true;
}

#endif
