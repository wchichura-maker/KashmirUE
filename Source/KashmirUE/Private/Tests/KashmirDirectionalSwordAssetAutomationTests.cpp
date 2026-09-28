#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirCombatantComponent.h"
#include "Combat/KashmirWeaponTraceComponent.h"
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Slot.h"
#include "EdGraph/EdGraph.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "KashmirCharacter.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordBaselineAssetsTest,
    "Kashmir.Combat.DirectionalSword.Assets.Baseline",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordBaselineAssetsTest::RunTest(
    const FString& Parameters)
{
    const TCHAR* ProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");
    const UKashmirDirectionalSwordProfile* Profile =
        LoadObject<UKashmirDirectionalSwordProfile>(nullptr, ProfilePath);
    TestNotNull(TEXT("Baseline directional sword profile loads"), Profile);
    if (Profile == nullptr)
    {
        return false;
    }

    FString Reason;
    TestTrue(TEXT("Baseline profile validates"), Profile->ValidateProfile(Reason));
    TestEqual(TEXT("All family-direction bindings are authored"),
        Profile->GestureConfig.ActionBindings.Num(), 16);
    TestEqual(TEXT("All resolved actions are authored"),
        Profile->Actions.Num(), 16);

    TSet<uint8> UniqueBindings;
    for (const FKashmirSwordActionBinding& Binding :
        Profile->GestureConfig.ActionBindings)
    {
        const uint8 Key =
            static_cast<uint8>(Binding.Family) * 8u +
            static_cast<uint8>(Binding.Direction);
        UniqueBindings.Add(Key);
    }
    TestEqual(TEXT("Bindings cover 16 unique family-direction pairs"),
        UniqueBindings.Num(), 16);

    for (const FKashmirSwordAuthoredAction& Action : Profile->Actions)
    {
        TestNotNull(
            *FString::Printf(TEXT("Montage loads for %s"),
                *Action.ActionId.ToString()),
            Action.Montage.LoadSynchronous());
        TestTrue(
            *FString::Printf(TEXT("%s uses contact delivery"),
                *Action.ActionId.ToString()),
            Action.CombatDefinition.Delivery ==
            EKashmirDeliveryType::Contact);
        TestTrue(
            *FString::Printf(TEXT("%s targets enemies"),
                *Action.ActionId.ToString()),
            Action.CombatDefinition.Target == EKashmirTargetType::Enemy);
        TestTrue(
            *FString::Printf(TEXT("%s resolves damage"),
                *Action.ActionId.ToString()),
            Action.CombatDefinition.Effects.Contains(
                EKashmirResolutionType::Damage));
        TestTrue(
            *FString::Printf(TEXT("%s has positive health damage"),
                *Action.ActionId.ToString()),
            Action.BaseDamage > 0.0f);
        TestTrue(
            *FString::Printf(TEXT("%s has positive guard damage"),
                *Action.ActionId.ToString()),
            Action.BaseGuardDamage > 0.0f);
    }

    const UInputAction* HoldAction = LoadObject<UInputAction>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Input/")
        TEXT("IA_SwordGestureHold.IA_SwordGestureHold"));
    TestNotNull(TEXT("Superseded sword gesture asset remains available for migration review"), HoldAction);
    if (HoldAction != nullptr)
    {
        TestEqual(TEXT("Legacy sword gesture action remains boolean"),
            HoldAction->ValueType, EInputActionValueType::Boolean);
    }

    const UInputMappingContext* MappingContext =
        LoadObject<UInputMappingContext>(
            nullptr,
            TEXT("/Game/KashmirAct/Characters/Player/Input/")
            TEXT("IMC_Player.IMC_Player"));
    TestNotNull(TEXT("Player input mapping context loads"), MappingContext);
    bool bHasMiddleMouseGestureMapping = false;
    if (MappingContext != nullptr)
    {
        for (const FEnhancedActionKeyMapping& Mapping :
            MappingContext->GetMappings())
        {
            bHasMiddleMouseGestureMapping |=
                Mapping.Action == HoldAction &&
                Mapping.Key == EKeys::MiddleMouseButton;
        }
    }
    TestFalse(TEXT("MMB no longer maps to the superseded sword gesture"),
        bHasMiddleMouseGestureMapping);

    UClass* CharacterClass = LoadObject<UClass>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Blueprints/")
        TEXT("BP_KashmirCharacter.BP_KashmirCharacter_C"));
    TestNotNull(TEXT("Player character Blueprint class loads"), CharacterClass);
    const AKashmirCharacter* CharacterDefaults = CharacterClass != nullptr
        ? Cast<AKashmirCharacter>(CharacterClass->GetDefaultObject())
        : nullptr;
    TestNotNull(TEXT("Player character defaults are valid"), CharacterDefaults);
    if (CharacterDefaults != nullptr)
    {
        TestNull(TEXT("Character exposes no gesture-input property"),
            FindFProperty<FObjectPropertyBase>(
                CharacterClass, TEXT("SwordGestureHoldAction")));
        TestTrue(TEXT("Directional sword component uses baseline profile"),
            CharacterDefaults->GetDirectionalSwordComponent()->GetProfile() ==
            Profile);
        TestNotNull(TEXT("Character owns an explicit weapon trace component"),
            CharacterDefaults->GetWeaponTraceComponent());
        TestNotNull(TEXT("Character owns the prototype sword mesh"),
            CharacterDefaults->GetSwordPrototypeMesh());
        TestNotNull(TEXT("Prototype sword exposes Weapon_Base"),
            CharacterDefaults->GetSwordTraceBase());
        TestNotNull(TEXT("Prototype sword exposes Weapon_Mid"),
            CharacterDefaults->GetSwordTraceMid());
        TestNotNull(TEXT("Prototype sword exposes Weapon_Tip"),
            CharacterDefaults->GetSwordTraceTip());

        const UKashmirCombatantComponent* Combatant =
            CharacterDefaults->GetCombatantComponent();
        TestNotNull(TEXT("Character owns the local combat state adapter"),
            Combatant);
        if (Combatant != nullptr)
        {
            TestEqual(TEXT("Prototype player has a stable combat identity"),
                Combatant->GetEntityId(), FName(TEXT("Player")));
            TestNotNull(TEXT("Prototype player has a hit-region map"),
                Combatant->GetHitRegionMap());
        }
    }

    UAnimBlueprint* AnimBlueprint = LoadObject<UAnimBlueprint>(
        nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Animation/")
        TEXT("ABP_KashmirCharacter.ABP_KashmirCharacter"));
    TestNotNull(TEXT("Player animation Blueprint loads"), AnimBlueprint);
    if (AnimBlueprint != nullptr)
    {
        TArray<UAnimGraphNode_Slot*> SlotNodes;
        TArray<UEdGraph*> Graphs;
        AnimBlueprint->GetAllGraphs(Graphs);
        for (UEdGraph* Graph : Graphs)
        {
            if (Graph != nullptr)
            {
                Graph->GetNodesOfClass(SlotNodes);
            }
        }
        TestTrue(TEXT("AnimBP evaluates the DefaultSlot montage track"),
            SlotNodes.ContainsByPredicate(
                [](const UAnimGraphNode_Slot* Node)
                {
                    return Node != nullptr &&
                        Node->Node.SlotName == TEXT("DefaultSlot");
                }));
    }

    return true;
}

#endif
