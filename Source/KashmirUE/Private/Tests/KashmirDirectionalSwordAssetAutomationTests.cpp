#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirCombatantComponent.h"
#include "Combat/KashmirWeaponTraceComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "AnimGraphNode_BlendListByBool.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_Slot.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "KashmirCharacter.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"


namespace
{
    UEdGraphPin* FindPosePin(UEdGraphNode* Node, EEdGraphPinDirection Direction)
    {
        if (Node == nullptr)
        {
            return nullptr;
        }

        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (Pin != nullptr && Pin->Direction == Direction &&
                Pin->PinType.PinCategory == TEXT("struct") &&
                Pin->PinType.PinSubCategoryObject.IsValid() &&
                Pin->PinType.PinSubCategoryObject->GetName() == TEXT("PoseLink"))
            {
                return Pin;
            }
        }
        return nullptr;
    }

    bool HasDirectPoseLink(UEdGraphNode* From, UEdGraphNode* To)
    {
        UEdGraphPin* Output = FindPosePin(From, EGPD_Output);
        UEdGraphPin* Input = FindPosePin(To, EGPD_Input);
        return Output != nullptr && Input != nullptr && Output->LinkedTo.Contains(Input);
    }

    bool HasAnyDirectPoseLink(UEdGraphNode* From, UEdGraphNode* To)
    {
        UEdGraphPin* Output = FindPosePin(From, EGPD_Output);
        if (Output == nullptr || To == nullptr)
        {
            return false;
        }
        return Output->LinkedTo.ContainsByPredicate(
            [To](const UEdGraphPin* Pin)
            {
                return Pin != nullptr && Pin->GetOwningNode() == To;
            });
    }

    bool HasPoseLinkToNamedInput(
        UEdGraphNode* From,
        UEdGraphNode* To,
        const FName InputPinName)
    {
        UEdGraphPin* Output = FindPosePin(From, EGPD_Output);
        UEdGraphPin* Input = To != nullptr
            ? To->FindPin(InputPinName, EGPD_Input)
            : nullptr;
        return Output != nullptr && Input != nullptr &&
            Output->LinkedTo.Contains(Input);
    }

    bool HasPoseLinkToLayeredOverlay(UEdGraphNode* From, UEdGraphNode* Layered)
    {
        UEdGraphPin* Output = FindPosePin(From, EGPD_Output);
        if (Output == nullptr || Layered == nullptr)
        {
            return false;
        }
        return Layered->Pins.ContainsByPredicate(
            [Output](const UEdGraphPin* Pin)
            {
                return Pin != nullptr && Pin->Direction == EGPD_Input &&
                    Pin->PinName != TEXT("BasePose") &&
                    Pin->PinType.PinCategory == TEXT("struct") &&
                    Pin->PinType.PinSubCategoryObject.IsValid() &&
                    Pin->PinType.PinSubCategoryObject->GetName() == TEXT("PoseLink") &&
                    Output->LinkedTo.Contains(Pin);
            });
    }

    bool HasDirectValueLink(
        UEdGraphNode* TargetNode,
        FName TargetPinName,
        FName SourcePinName)
    {
        const UEdGraphPin* TargetPin = TargetNode != nullptr
            ? TargetNode->FindPin(TargetPinName, EGPD_Input) : nullptr;
        if (TargetPin == nullptr || TargetPin->LinkedTo.Num() != 1)
        {
            return false;
        }
        const UEdGraphPin* SourcePin = TargetPin->LinkedTo[0];
        return SourcePin != nullptr &&
            SourcePin->Direction == EGPD_Output &&
            SourcePin->PinName == SourcePinName &&
            SourcePin->GetOwningNode() != nullptr &&
            SourcePin->GetOwningNode()->GetClass()->GetName() == TEXT("K2Node_VariableGet");
    }

    TMap<FName, FName> ReadControlRigInputMapping(UEdGraphNode* ControlRigNode)
    {
        TMap<FName, FName> Result;
        const FStructProperty* NodeProperty = ControlRigNode != nullptr
            ? FindFProperty<FStructProperty>(ControlRigNode->GetClass(), TEXT("Node"))
            : nullptr;
        const FMapProperty* MapProperty = NodeProperty != nullptr
            ? FindFProperty<FMapProperty>(NodeProperty->Struct, TEXT("InputMapping"))
            : nullptr;
        if (MapProperty == nullptr)
        {
            return Result;
        }

        void* NodeValue = NodeProperty->ContainerPtrToValuePtr<void>(ControlRigNode);
        FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(NodeValue));
        for (int32 Index = 0; Index < MapHelper.GetMaxIndex(); ++Index)
        {
            if (MapHelper.IsValidIndex(Index))
            {
                const FName* Key = reinterpret_cast<const FName*>(MapHelper.GetKeyPtr(Index));
                const FName* Value = reinterpret_cast<const FName*>(MapHelper.GetValuePtr(Index));
                Result.Add(*Key, *Value);
            }
        }
        return Result;
    }
}


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
        UAnimMontage* Montage = Action.Montage.LoadSynchronous();
        TestNotNull(
            *FString::Printf(TEXT("Montage loads for %s"),
                *Action.ActionId.ToString()),
            Montage);
        if (Montage != nullptr)
        {
            TestTrue(
                *FString::Printf(TEXT("%s montage remains on DefaultSlot"),
                    *Action.ActionId.ToString()),
                Montage->SlotAnimTracks.ContainsByPredicate(
                    [](const FSlotAnimationTrack& Track)
                    {
                        return Track.SlotName == TEXT("DefaultSlot");
                    }));
        }
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
        const UAnimInstance* AnimDefaults = AnimBlueprint->GeneratedClass != nullptr
            ? Cast<UAnimInstance>(AnimBlueprint->GeneratedClass->GetDefaultObject())
            : nullptr;
        TestNotNull(TEXT("Player AnimInstance defaults load"), AnimDefaults);
        if (AnimDefaults != nullptr)
        {
            TestEqual(TEXT("AnimBP remains Root Motion From Montages Only"),
                AnimDefaults->RootMotionMode.GetValue(),
                ERootMotionMode::RootMotionFromMontagesOnly);
        }

        TArray<UAnimGraphNode_Slot*> SlotNodes;
        UEdGraphNode* ControlRigNode = nullptr;
        UAnimGraphNode_LayeredBoneBlend* UpperBodyBlend = nullptr;
        UAnimGraphNode_BlendListByBool* MovementIntentRoute = nullptr;
        UEdGraphNode* StateMachineNode = nullptr;
        UEdGraphNode* RootNode = nullptr;
        TArray<UEdGraph*> Graphs;
        AnimBlueprint->GetAllGraphs(Graphs);
        for (UEdGraph* Graph : Graphs)
        {
            if (Graph != nullptr)
            {
                Graph->GetNodesOfClass(SlotNodes);
                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_ControlRig"))
                    {
                        ControlRigNode = Node;
                    }
                    else if (UAnimGraphNode_LayeredBoneBlend* Blend =
                        Cast<UAnimGraphNode_LayeredBoneBlend>(Node))
                    {
                        UpperBodyBlend = Blend;
                    }
                    else if (UAnimGraphNode_BlendListByBool* BlendBool =
                        Cast<UAnimGraphNode_BlendListByBool>(Node))
                    {
                        MovementIntentRoute = BlendBool;
                    }
                    else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_StateMachine"))
                    {
                        StateMachineNode = Node;
                    }
                    else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_Root"))
                    {
                        RootNode = Node;
                    }
                }
            }
        }
        UAnimGraphNode_Slot* DefaultSlot = nullptr;
        for (UAnimGraphNode_Slot* Node : SlotNodes)
        {
            if (Node != nullptr && Node->Node.SlotName == TEXT("DefaultSlot"))
            {
                DefaultSlot = Node;
                break;
            }
        }
        // Resolve topology only inside the main AnimGraph that owns DefaultSlot;
        // state-machine subgraphs may contain unrelated animation nodes.
        if (DefaultSlot != nullptr && DefaultSlot->GetGraph() != nullptr)
        {
            UpperBodyBlend = nullptr;
            MovementIntentRoute = nullptr;
            ControlRigNode = nullptr;
            StateMachineNode = nullptr;
            RootNode = nullptr;
            for (UEdGraphNode* Node : DefaultSlot->GetGraph()->Nodes)
            {
                if (UAnimGraphNode_LayeredBoneBlend* Blend =
                    Cast<UAnimGraphNode_LayeredBoneBlend>(Node))
                {
                    UpperBodyBlend = Blend;
                }
                else if (UAnimGraphNode_BlendListByBool* BlendBool =
                    Cast<UAnimGraphNode_BlendListByBool>(Node))
                {
                    MovementIntentRoute = BlendBool;
                }
                else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_ControlRig"))
                {
                    ControlRigNode = Node;
                }
                else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_StateMachine"))
                {
                    StateMachineNode = Node;
                }
                else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_Root"))
                {
                    RootNode = Node;
                }
            }
        }
        TestNotNull(TEXT("AnimBP evaluates the DefaultSlot montage track"), DefaultSlot);
        TestNotNull(TEXT("AnimBP contains the sword Control Rig node"), ControlRigNode);
        TestNotNull(TEXT("Stationary sword montage uses an upper-body layered blend"), UpperBodyBlend);
        TestNotNull(TEXT("Movement Intent selects the montage body route"), MovementIntentRoute);
        TestTrue(TEXT("Locomotion state machine feeds DefaultSlot"),
            HasAnyDirectPoseLink(StateMachineNode, DefaultSlot));
        TestTrue(TEXT("Locomotion feeds the layered blend BasePose"),
            HasPoseLinkToNamedInput(
                StateMachineNode, UpperBodyBlend, TEXT("BasePose")));
        TestTrue(TEXT("DefaultSlot feeds the layered non-base overlay pose"),
            HasPoseLinkToLayeredOverlay(DefaultSlot, UpperBodyBlend));
        TestTrue(TEXT("Stationary layered pose feeds bool child 1 (false)"),
            HasPoseLinkToNamedInput(
                UpperBodyBlend, MovementIntentRoute, TEXT("BlendPose_1")));
        TestTrue(TEXT("FullBody slot pose feeds bool child 0 (true)"),
            HasPoseLinkToNamedInput(
                DefaultSlot, MovementIntentRoute, TEXT("BlendPose_0")));
        TestTrue(TEXT("Sword Control Rig evaluates downstream of Movement Intent"),
            HasDirectPoseLink(MovementIntentRoute, ControlRigNode));
        TestTrue(TEXT("Post-montage Control Rig feeds the final output pose"),
            HasDirectPoseLink(ControlRigNode, RootNode));
        if (UpperBodyBlend != nullptr)
        {
            TestEqual(TEXT("Upper-body blend has one montage layer"),
                UpperBodyBlend->Node.LayerSetup.Num(), 1);
            TestTrue(TEXT("Upper-body blend uses mesh-space rotation"),
                UpperBodyBlend->Node.bMeshSpaceRotationBlend);
            TestEqual(TEXT("Upper-body montage layer weight is one"),
                UpperBodyBlend->Node.BlendWeights.Num(), 1);
            if (UpperBodyBlend->Node.BlendWeights.Num() == 1)
            {
                TestEqual(TEXT("Upper-body montage layer has full weight"),
                    UpperBodyBlend->Node.BlendWeights[0], 1.0f);
            }
            TestEqual(TEXT("Upper-body curve blend uses Override"),
                UpperBodyBlend->Node.CurveBlendOption.GetValue(),
                ECurveBlendOption::Override);
            TestTrue(TEXT("Root motion weight follows the root bone"),
                UpperBodyBlend->Node.bBlendRootMotionBasedOnRootBone);
            TestTrue(TEXT("Upper-body blend has a branch filter"),
                UpperBodyBlend->Node.LayerSetup.IsValidIndex(0) &&
                UpperBodyBlend->Node.LayerSetup[0].BranchFilters.Num() == 1);
            if (UpperBodyBlend->Node.LayerSetup.IsValidIndex(0) &&
                UpperBodyBlend->Node.LayerSetup[0].BranchFilters.Num() == 1)
            {
                const FBranchFilter& Filter =
                    UpperBodyBlend->Node.LayerSetup[0].BranchFilters[0];
                TestEqual(TEXT("Stationary attack overlay starts at spine_01"),
                    Filter.BoneName, FName(TEXT("spine_01")));
                TestEqual(TEXT("Upper-body branch reaches full weight immediately"),
                    Filter.BlendDepth, 1);
            }
        }

        const TMap<FName, FName> InputMapping = ReadControlRigInputMapping(ControlRigNode);
        TestEqual(TEXT("Control Rig no longer depends on animation-curve mappings"),
            InputMapping.Num(), 0);

        static const TPair<FName, FName> DirectBindings[] =
        {
            { TEXT("LeadHandOffsetX"), TEXT("SwordLeadHandOffsetX") },
            { TEXT("LeadHandOffsetY"), TEXT("SwordLeadHandOffsetY") },
            { TEXT("LeadHandOffsetZ"), TEXT("SwordLeadHandOffsetZ") },
            { TEXT("SupportHandOffsetX"), TEXT("SwordSupportHandOffsetX") },
            { TEXT("SupportHandOffsetY"), TEXT("SwordSupportHandOffsetY") },
            { TEXT("SupportHandOffsetZ"), TEXT("SwordSupportHandOffsetZ") },
            { TEXT("AimPitch"), TEXT("SwordAimPitch") },
            { TEXT("AimYaw"), TEXT("SwordAimYaw") },
            { TEXT("AimRoll"), TEXT("SwordAimRoll") },
            { TEXT("BodyLean"), TEXT("SwordBodyLean") },
            { TEXT("SwordPoseAlpha"), TEXT("SwordPoseAlpha") },
            { TEXT("LeftFootLockAlpha"), TEXT("SwordLeftFootLockAlpha") },
            { TEXT("RightFootLockAlpha"), TEXT("SwordRightFootLockAlpha") }
        };
        for (const TPair<FName, FName>& Binding : DirectBindings)
        {
            TestTrue(
                *FString::Printf(TEXT("Control Rig %s consumes AnimInstance %s directly"),
                    *Binding.Key.ToString(), *Binding.Value.ToString()),
                HasDirectValueLink(ControlRigNode, Binding.Key, Binding.Value));
        }
    }

    return true;
}

#endif
