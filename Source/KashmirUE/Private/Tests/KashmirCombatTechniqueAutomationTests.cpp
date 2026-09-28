#include "Misc/AutomationTest.h"

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "UObject/UnrealType.h"


#if WITH_DEV_AUTOMATION_TESTS

namespace
{
    FKashmirCombatTechniqueDefinition MakeTechnique(
        const FName TechniqueId,
        const FName ActionId,
        const FName WeaponFamily,
        const EKashmirAttackDirection Direction,
        const EKashmirAttackShape Shape)
    {
        FKashmirCombatTechniqueDefinition Result;
        Result.TechniqueId = TechniqueId;
        Result.ActionId = ActionId;
        Result.WeaponFamily = WeaponFamily;
        Result.TechniqueFamily = Shape == EKashmirAttackShape::Thrust
            ? TEXT("Technique.Thrust")
            : TEXT("Technique.Cut");
        Result.AttackDirection = Direction;
        Result.AttackShape = Shape;
        Result.RuntimeDefinition.ActionId = ActionId;
        Result.RuntimeDefinition.StartupDuration = 0.10f;
        Result.RuntimeDefinition.ActiveDuration = 0.15f;
        Result.RuntimeDefinition.RecoveryDuration = 0.20f;
        Result.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Result.BaseDamage = 20.0f;
        Result.BaseGuardDamage = 10.0f;
        Result.MasteryChannels.Add(Shape == EKashmirAttackShape::Thrust
            ? TEXT("Aptitude.Thrusting")
            : TEXT("Aptitude.Cutting"));
        return Result;
    }


    UKashmirWeaponCombatStyle* MakeStyle(
        const FName StyleId,
        const FName WeaponFamily,
        const FKashmirCombatTechniqueDefinition& Technique)
    {
        UKashmirWeaponCombatStyle* Style =
            NewObject<UKashmirWeaponCombatStyle>();
        Style->StyleId = StyleId;
        Style->WeaponFamily = WeaponFamily;
        Style->Techniques.Add(Technique);
        FKashmirTechniqueSlotBinding Binding;
        Binding.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
        Binding.TechniqueId = Technique.TechniqueId;
        Style->SlotBindings.Add(Binding);
        return Style;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirWeaponCombatGrammarTwoStylesTest,
    "Kashmir.Combat.Technique.TwoWeaponGrammars",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirWeaponCombatGrammarTwoStylesTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Sword = MakeStyle(
        TEXT("Style.Sword.Versatile"), TEXT("Weapon.Sword"),
        MakeTechnique(
            TEXT("Technique.Sword.CrossSlash"), TEXT("Sword.Direct.Right"),
            TEXT("Weapon.Sword"), EKashmirAttackDirection::LeftToRight,
            EKashmirAttackShape::Slash));
    UKashmirWeaponCombatStyle* Spear = MakeStyle(
        TEXT("Style.Spear.Interception"), TEXT("Weapon.Spear"),
        MakeTechnique(
            TEXT("Technique.Spear.QuickThrust"), TEXT("Spear.QuickThrust"),
            TEXT("Weapon.Spear"), EKashmirAttackDirection::Thrust,
            EKashmirAttackShape::Thrust));

    FKashmirTechniqueRequest Request;
    Request.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
    FKashmirTechniqueActionPlan SwordPlan;
    FKashmirTechniqueActionPlan SpearPlan;
    FString Reason;
    TestTrue(TEXT("Sword slot resolves"),
        Sword->ResolveTechnique(Request, SwordPlan, Reason));
    TestTrue(TEXT("Spear slot resolves through the same core"),
        Spear->ResolveTechnique(Request, SpearPlan, Reason));
    TestNotEqual(TEXT("The same slot resolves weapon-specific technique ids"),
        SwordPlan.Technique.TechniqueId, SpearPlan.Technique.TechniqueId);
    TestNotEqual(TEXT("The same slot resolves weapon-specific actions"),
        SwordPlan.ActionRequest.ActionId, SpearPlan.ActionRequest.ActionId);
    TestEqual(TEXT("Both grammars use the same data-driven style class"),
        Sword->GetClass(), Spear->GetClass());
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueRequestAuthorityTest,
    "Kashmir.Combat.Technique.SharedRequestAuthority",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirTechniqueRequestAuthorityTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = MakeStyle(
        TEXT("Style.Sword.Versatile"), TEXT("Weapon.Sword"),
        MakeTechnique(
            TEXT("Technique.Sword.CrossSlash"), TEXT("Sword.Direct.Right"),
            TEXT("Weapon.Sword"), EKashmirAttackDirection::LeftToRight,
            EKashmirAttackShape::Slash));

    FKashmirTechniqueRequest PlayerRequest;
    PlayerRequest.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
    PlayerRequest.Source = EKashmirTechniqueRequestSource::Player;
    FKashmirTechniqueRequest AIRequest = PlayerRequest;
    AIRequest.Source = EKashmirTechniqueRequestSource::AI;

    FKashmirTechniqueActionPlan PlayerPlan;
    FKashmirTechniqueActionPlan AIPlan;
    FString Reason;
    TestTrue(TEXT("Player-like request resolves"),
        Style->ResolveTechnique(PlayerRequest, PlayerPlan, Reason));
    TestTrue(TEXT("AI-like request resolves"),
        Style->ResolveTechnique(AIRequest, AIPlan, Reason));
    TestEqual(TEXT("Sources converge on identical ActionId semantics"),
        PlayerPlan.ActionRequest.ActionId, AIPlan.ActionRequest.ActionId);
    TestEqual(TEXT("Sources converge on authored direction semantics"),
        PlayerPlan.ActionRequest.Direction, AIPlan.ActionRequest.Direction);
    TestEqual(TEXT("Direction metadata reaches ActionRequest"),
        PlayerPlan.ActionRequest.Direction, FVector2D::UnitX());
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueProgressionBoundaryTest,
    "Kashmir.Combat.Technique.ProgressionBoundary",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirTechniqueProgressionBoundaryTest::RunTest(const FString& Parameters)
{
    const UScriptStruct* Struct =
        FKashmirCombatTechniqueDefinition::StaticStruct();
    TestNull(TEXT("Technique definitions never award XP directly"),
        Struct->FindPropertyByName(TEXT("Experience")));
    TestNull(TEXT("Technique definitions contain no execution XP grant"),
        Struct->FindPropertyByName(TEXT("ExperienceReward")));
    TestNotNull(TEXT("Technique definitions expose evidence-compatible mastery channels"),
        Struct->FindPropertyByName(TEXT("MasteryChannels")));
    TestNotNull(TEXT("Technique definitions support Ultimate Lineage metadata"),
        Struct->FindPropertyByName(TEXT("UltimateLineageId")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueSwordBridgeTest,
    "Kashmir.Combat.Technique.SwordActionRuntimeBridge",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirTechniqueSwordBridgeTest::RunTest(const FString& Parameters)
{
    const UKashmirDirectionalSwordProfile* LoadedProfile =
        LoadObject<UKashmirDirectionalSwordProfile>(
            nullptr,
            TEXT("/Game/KashmirAct/Combat/DirectionalSword/"
                 "DA_KashmirDirectionalSword_Baseline."
                 "DA_KashmirDirectionalSword_Baseline"));
    TestNotNull(TEXT("Existing sword profile loads"), LoadedProfile);
    if (LoadedProfile == nullptr || LoadedProfile->Actions.IsEmpty())
    {
        return false;
    }

    const FKashmirSwordAuthoredAction& ExistingAction =
        LoadedProfile->Actions[0];
    FKashmirCombatTechniqueDefinition Technique = MakeTechnique(
        TEXT("Technique.Sword.BaselineSlot1"), ExistingAction.ActionId,
        TEXT("Weapon.Sword"), EKashmirAttackDirection::LeftToRight,
        EKashmirAttackShape::Slash);
    Technique.Montage = ExistingAction.Montage;
    Technique.MontageSection = ExistingAction.MontageSection;
    Technique.PlayRate = ExistingAction.PlayRate;
    Technique.RuntimeDefinition = ExistingAction.BuildRuntimeDefinition();
    Technique.CombatDefinition = ExistingAction.CombatDefinition;
    Technique.BaseDamage = ExistingAction.BaseDamage;
    Technique.BaseGuardDamage = ExistingAction.BaseGuardDamage;

    UKashmirWeaponCombatStyle* Style = MakeStyle(
        TEXT("Style.Sword.Baseline"), TEXT("Weapon.Sword"), Technique);
    UKashmirDirectionalSwordComponent* Component =
        NewObject<UKashmirDirectionalSwordComponent>();
    Component->SetProfile(
        const_cast<UKashmirDirectionalSwordProfile*>(LoadedProfile));

    FKashmirTechniqueRequest Request;
    Request.Slot = EKashmirTechniqueSlot::TechniqueSlot1;
    FString Reason;
    TestTrue(TEXT("Technique request starts the existing sword runtime"),
        Component->StartTechniqueRequest(Request, Style, Reason));
    TestEqual(TEXT("Technique bridge preserves existing authored action"),
        Component->GetRuntimeState().ActionId, ExistingAction.ActionId);
    TestTrue(TEXT("Existing sword montage remains bound"),
        Component->GetActivePlan().Montage == ExistingAction.Montage);
    return true;
}

#endif
