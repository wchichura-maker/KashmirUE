#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "AnimNodes/AnimNode_BlendListByBool.h"
#include "AnimGraphNode_BlendListByBool.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_Slot.h"
#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Components/SkeletalMeshComponent.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "KashmirAnimInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace
{
    struct FBlendListByBoolSemanticProbe : FAnimNode_BlendListByBool
    {
        int32 ResolveChildIndex(const bool bValue)
        {
            FBoolProperty* ActiveValueProperty = FindFProperty<FBoolProperty>(
                FAnimNode_BlendListByBool::StaticStruct(), TEXT("bActiveValue"));
            check(ActiveValueProperty != nullptr);
            ActiveValueProperty->SetPropertyValue_InContainer(this, bValue);
            return GetActiveChildIndex();
        }
    };

    constexpr const TCHAR* MovementIntentStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* MovementIntentProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");
    const FName FullBodyId(TEXT("Technique.Sword.Test.FullBody"));
    const FName MovementIntentQuickId(TEXT("Technique.Sword.Diagonal.Rising.Quick"));
    const FName MovementIntentWideId(TEXT("Technique.Sword.Diagonal.Rising.Wide"));

    UKashmirWeaponCombatStyle* LoadMovementIntentStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(nullptr, MovementIntentStylePath);
    }

    UKashmirDirectionalSwordProfile* LoadMovementIntentProfile()
    {
        return LoadObject<UKashmirDirectionalSwordProfile>(nullptr, MovementIntentProfilePath);
    }

    const FKashmirCombatTechniqueDefinition* FindMovementIntentTechnique(
        const UKashmirWeaponCombatStyle* Style,
        const FName Id)
    {
        return Style != nullptr
            ? Style->Techniques.FindByPredicate(
                [Id](const FKashmirCombatTechniqueDefinition& Item)
                {
                    return Item.TechniqueId == Id;
                })
            : nullptr;
    }

    bool ResolveMovementIntentPlanForTechnique(
        UKashmirWeaponCombatStyle* SourceStyle,
        UKashmirDirectionalSwordProfile* Profile,
        const FName TechniqueId,
        FKashmirSwordActionPlan& OutPlan,
        FString& OutReason)
    {
        if (SourceStyle == nullptr || Profile == nullptr || SourceStyle->SlotBindings.IsEmpty())
        {
            OutReason = TEXT("missing Movement Intent fixture data");
            return false;
        }
        UKashmirWeaponCombatStyle* TestStyle =
            DuplicateObject<UKashmirWeaponCombatStyle>(SourceStyle, GetTransientPackage());
        TestStyle->SlotBindings[0].TechniqueId = TechniqueId;
        UKashmirDirectionalSwordComponent* Sword =
            NewObject<UKashmirDirectionalSwordComponent>();
        Sword->SetProfile(Profile);
        FKashmirTechniqueRequest Request;
        Request.Slot = TestStyle->SlotBindings[0].Slot;
        if (!Sword->StartTechniqueRequest(Request, TestStyle, OutReason))
        {
            return false;
        }
        OutPlan = Sword->GetActivePlan();
        return true;
    }

    UEdGraphPin* FindMovementIntentPosePin(UEdGraphNode* Node, EEdGraphPinDirection Direction, int32 Index = 0)
    {
        int32 Found = 0;
        if (Node != nullptr)
        {
            for (UEdGraphPin* Pin : Node->Pins)
            {
                if (Pin != nullptr && Pin->Direction == Direction &&
                    Pin->PinType.PinCategory == TEXT("struct") &&
                    Pin->PinType.PinSubCategoryObject.IsValid() &&
                    Pin->PinType.PinSubCategoryObject->GetName() == TEXT("PoseLink"))
                {
                    if (Found++ == Index)
                    {
                        return Pin;
                    }
                }
            }
        }
        return nullptr;
    }

    bool HasMovementIntentLink(UEdGraphPin* From, UEdGraphPin* To)
    {
        return From != nullptr && To != nullptr && From->LinkedTo.Contains(To);
    }

    int32 ResolveMovementIntentBoolChildIndex(const bool bValue)
    {
        FBlendListByBoolSemanticProbe Probe;
        return Probe.ResolveChildIndex(bValue);
    }

    UEdGraphPin* FindMovementIntentRoutePosePinForValue(
        UAnimGraphNode_BlendListByBool* Route,
        const bool bValue)
    {
        const int32 ChildIndex = ResolveMovementIntentBoolChildIndex(bValue);
        return Route != nullptr
            ? Route->FindPin(*FString::Printf(TEXT("BlendPose_%d"), ChildIndex))
            : nullptr;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementIntentContractTest,
    "Kashmir.Combat.MovementIntent.Contract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementIntentContractTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition Definition;
    const FKashmirSwordActionPlan Plan;
    TestEqual(TEXT("Technique default is safely Stationary"),
        Definition.MovementIntent, EKashmirMovementIntent::Stationary);
    TestEqual(TEXT("Action plan default is safely Stationary"),
        Plan.MovementIntent, EKashmirMovementIntent::Stationary);
    const UEnum* Enum = StaticEnum<EKashmirMovementIntent>();
    TestNotNull(TEXT("Movement Intent enum is reflected"), Enum);
    if (Enum != nullptr)
    {
        TestEqual(TEXT("v0.1 exposes exactly two authored values"),
            Enum->NumEnums() - 1, 2);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementIntentTechniqueRoutesTest,
    "Kashmir.Combat.MovementIntent.TechniqueRoutes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementIntentTechniqueRoutesTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = LoadMovementIntentStyle();
    UKashmirDirectionalSwordProfile* Profile = LoadMovementIntentProfile();
    const FKashmirCombatTechniqueDefinition* FullBody = FindMovementIntentTechnique(Style, FullBodyId);
    TestNotNull(TEXT("FullBody proof Technique exists"), FullBody);
    if (Style == nullptr || Profile == nullptr || FullBody == nullptr || Style->SlotBindings.IsEmpty())
    {
        return false;
    }
    const FName StationaryId = Style->SlotBindings[0].TechniqueId;
    FKashmirSwordActionPlan StationaryPlan;
    FKashmirSwordActionPlan FullBodyPlan;
    FString Reason;
    TestTrue(TEXT("Stationary Technique resolves"),
        ResolveMovementIntentPlanForTechnique(Style, Profile, StationaryId, StationaryPlan, Reason));
    Reason.Reset();
    TestTrue(TEXT("FullBody Technique resolves through the same runtime"),
        ResolveMovementIntentPlanForTechnique(Style, Profile, FullBodyId, FullBodyPlan, Reason));
    TestEqual(TEXT("Stationary route reaches the action plan"),
        StationaryPlan.MovementIntent, EKashmirMovementIntent::Stationary);
    TestEqual(TEXT("FullBody route reaches the action plan"),
        FullBodyPlan.MovementIntent, EKashmirMovementIntent::FullBody);
    TestEqual(TEXT("Movement Intent does not change ActionId authority"),
        FullBodyPlan.RuntimeDefinition.ActionId, StationaryPlan.RuntimeDefinition.ActionId);
    TestEqual(TEXT("Movement Intent does not change damage"),
        FullBodyPlan.BaseDamage, StationaryPlan.BaseDamage);
    TestEqual(TEXT("Movement Intent does not change WeaponTrace timing"),
        FullBodyPlan.RuntimeDefinition.ActiveDuration,
        StationaryPlan.RuntimeDefinition.ActiveDuration);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementIntentBaselineTest,
    "Kashmir.Combat.MovementIntent.Baseline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementIntentBaselineTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadMovementIntentStyle();
    TestNotNull(TEXT("Sword baseline loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    for (const FKashmirTechniqueSlotBinding& Binding : Style->SlotBindings)
    {
        const FKashmirCombatTechniqueDefinition* Technique =
            FindMovementIntentTechnique(Style, Binding.TechniqueId);
        TestNotNull(*FString::Printf(TEXT("Bound slot %d resolves"), static_cast<int32>(Binding.Slot)), Technique);
        if (Technique != nullptr)
        {
            TestEqual(*FString::Printf(TEXT("Bound slot %d remains Stationary"), static_cast<int32>(Binding.Slot)),
                Technique->MovementIntent, EKashmirMovementIntent::Stationary);
        }
    }
    for (const FName Id : { MovementIntentQuickId, MovementIntentWideId })
    {
        const FKashmirCombatTechniqueDefinition* Technique = FindMovementIntentTechnique(Style, Id);
        TestNotNull(*FString::Printf(TEXT("%s exists"), *Id.ToString()), Technique);
        if (Technique != nullptr)
        {
            TestEqual(*FString::Printf(TEXT("%s remains Stationary"), *Id.ToString()),
                Technique->MovementIntent, EKashmirMovementIntent::Stationary);
        }
    }
    const FKashmirCombatTechniqueDefinition* FullBody = FindMovementIntentTechnique(Style, FullBodyId);
    TestNotNull(TEXT("Unbound FullBody proof exists"), FullBody);
    if (FullBody != nullptr)
    {
        TestEqual(TEXT("Proof Technique is FullBody"),
            FullBody->MovementIntent, EKashmirMovementIntent::FullBody);
    }
    TestFalse(TEXT("FullBody proof is not bound to a playable slot"),
        Style->SlotBindings.ContainsByPredicate(
            [](const FKashmirTechniqueSlotBinding& Binding)
            {
                return Binding.TechniqueId == FullBodyId;
            }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementIntentAnimBridgeTest,
    "Kashmir.Combat.MovementIntent.AnimBridge",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementIntentAnimBridgeTest::RunTest(const FString& Parameters)
{
    USkeletalMeshComponent* Mesh = NewObject<USkeletalMeshComponent>();
    UKashmirAnimInstance* AnimInstance = NewObject<UKashmirAnimInstance>(Mesh);
    const FBoolProperty* Selector = FindFProperty<FBoolProperty>(
        UKashmirAnimInstance::StaticClass(), TEXT("bUseFullBodySwordMontage"));
    TestNotNull(TEXT("Transient AnimGraph selector is reflected"), Selector);
    AnimInstance->SetSwordMovementIntent(EKashmirMovementIntent::FullBody);
    TestEqual(TEXT("AnimInstance stores FullBody intent"),
        AnimInstance->GetSwordMovementIntent(), EKashmirMovementIntent::FullBody);
    if (Selector != nullptr)
    {
        TestTrue(TEXT("FullBody enables the full-body route"),
            Selector->GetPropertyValue_InContainer(AnimInstance));
    }
    AnimInstance->SetSwordMovementIntent(EKashmirMovementIntent::Stationary);
    if (Selector != nullptr)
    {
        TestFalse(TEXT("Stationary restores the upper-body route"),
            Selector->GetPropertyValue_InContainer(AnimInstance));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementIntentAnimGraphTest,
    "Kashmir.Combat.MovementIntent.AnimGraphStructuralRoute",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementIntentAnimGraphTest::RunTest(const FString& Parameters)
{
    UAnimBlueprint* Blueprint = LoadObject<UAnimBlueprint>(nullptr,
        TEXT("/Game/KashmirAct/Characters/Player/Animation/ABP_KashmirCharacter.ABP_KashmirCharacter"));
    TestNotNull(TEXT("AnimBP loads"), Blueprint);
    if (Blueprint == nullptr)
    {
        return false;
    }
    UAnimGraphNode_Slot* Slot = nullptr;
    UAnimGraphNode_LayeredBoneBlend* Layered = nullptr;
    UAnimGraphNode_BlendListByBool* Route = nullptr;
    UEdGraphNode* Rig = nullptr;
    UEdGraphNode* Selector = nullptr;
    UEdGraph* MainAnimGraph = nullptr;
    TArray<UEdGraph*> Graphs;
    Blueprint->GetAllGraphs(Graphs);
    for (UEdGraph* Graph : Graphs)
    {
        if (Graph == nullptr) continue;
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UAnimGraphNode_Slot* SlotNode = Cast<UAnimGraphNode_Slot>(Node);
                SlotNode != nullptr && SlotNode->Node.SlotName == TEXT("DefaultSlot"))
            {
                Slot = SlotNode;
                MainAnimGraph = Graph;
            }
        }
    }
    if (MainAnimGraph != nullptr)
    {
        for (UEdGraphNode* Node : MainAnimGraph->Nodes)
        {
            if (UAnimGraphNode_LayeredBoneBlend* LayeredNode = Cast<UAnimGraphNode_LayeredBoneBlend>(Node)) Layered = LayeredNode;
            else if (UAnimGraphNode_BlendListByBool* RouteNode = Cast<UAnimGraphNode_BlendListByBool>(Node)) Route = RouteNode;
            else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("AnimGraphNode_ControlRig")) Rig = Node;
            else if (Node != nullptr && Node->GetClass()->GetName() == TEXT("K2Node_VariableGet") &&
                Node->FindPin(TEXT("bUseFullBodySwordMontage")) != nullptr) Selector = Node;
        }
    }
    TestNotNull(TEXT("Stationary layered route exists"), Layered);
    TestNotNull(TEXT("FullBody selector exists"), Route);
    TestNotNull(TEXT("Movement Intent selector source exists"), Selector);
    TestEqual(TEXT("UE bool false selects child 1"), ResolveMovementIntentBoolChildIndex(false), 1);
    TestEqual(TEXT("UE bool true selects child 0"), ResolveMovementIntentBoolChildIndex(true), 0);
    TestTrue(TEXT("Stationary false evaluates the layered route"),
        HasMovementIntentLink(FindMovementIntentPosePin(Layered, EGPD_Output),
            FindMovementIntentRoutePosePinForValue(Route, false)));
    TestTrue(TEXT("FullBody true evaluates the raw DefaultSlot route"),
        HasMovementIntentLink(FindMovementIntentPosePin(Slot, EGPD_Output),
            FindMovementIntentRoutePosePinForValue(Route, true)));
    TestTrue(TEXT("Route selector remains upstream of Control Rig"),
        HasMovementIntentLink(FindMovementIntentPosePin(Route, EGPD_Output),
            FindMovementIntentPosePin(Rig, EGPD_Input)));
    TestTrue(TEXT("Movement Intent drives the bool selector"),
        Selector != nullptr && Route != nullptr &&
        HasMovementIntentLink(Selector->FindPin(TEXT("bUseFullBodySwordMontage")),
            Route->FindPin(TEXT("bActiveValue"))));
    const UAnimInstance* Defaults = Blueprint->GeneratedClass != nullptr
        ? Cast<UAnimInstance>(Blueprint->GeneratedClass->GetDefaultObject())
        : nullptr;
    TestNotNull(TEXT("AnimBP defaults load"), Defaults);
    if (Defaults != nullptr)
    {
        TestEqual(TEXT("Root Motion policy remains montage-only"),
            Defaults->RootMotionMode.GetValue(), ERootMotionMode::RootMotionFromMontagesOnly);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMovementIntentNoSpecificBranchTest,
    "Kashmir.Combat.MovementIntent.NoTechniqueSpecificBranching",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirMovementIntentNoSpecificBranchTest::RunTest(const FString& Parameters)
{
    FString Source;
    const FString Path = FPaths::Combine(FPaths::ProjectDir(),
        TEXT("Source/KashmirUE/Private/Combat/KashmirDirectionalSwordComponent.cpp"));
    TestTrue(TEXT("Sword runtime source is readable"), FFileHelper::LoadFileToString(Source, *Path));
    TestFalse(TEXT("Runtime has no FullBody proof ID branch"), Source.Contains(FullBodyId.ToString()));
    TestFalse(TEXT("Runtime has no Quick branch"), Source.Contains(TEXT("Rising.Quick")));
    TestFalse(TEXT("Runtime has no Wide branch"), Source.Contains(TEXT("Rising.Wide")));
    return true;
}

#endif
