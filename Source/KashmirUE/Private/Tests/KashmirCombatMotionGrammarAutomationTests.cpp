#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirPhysicalReactionResolver.h"
#include "Contracts/KashmirActionContracts.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"


namespace
{
    constexpr const TCHAR* CombatMotionGrammarStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    const FName CombatMotionGrammarStepForwardId(
        TEXT("Technique.Sword.Test.StepForward"));

    const UKashmirWeaponCombatStyle* LoadCombatMotionGrammarStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(
            nullptr, CombatMotionGrammarStylePath);
    }

    const FKashmirCombatTechniqueDefinition* FindCombatMotionStepForward(
        const UKashmirWeaponCombatStyle* Style)
    {
        return Style != nullptr
            ? Style->Techniques.FindByPredicate(
                [](const FKashmirCombatTechniqueDefinition& Technique)
                {
                    return Technique.TechniqueId ==
                        CombatMotionGrammarStepForwardId;
                })
            : nullptr;
    }

    FString LoadCombatMotionSource(const TCHAR* RelativePath)
    {
        FString Source;
        FFileHelper::LoadFileToString(
            Source,
            *FPaths::Combine(FPaths::ProjectDir(), RelativePath));
        return Source;
    }
}


#define KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(ClassName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
        ClassName, \
        "Kashmir.Combat.CombatMotionGrammar." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionVoluntaryTranslationIntentTest,
    "VoluntaryTranslationIsSeparateFromMovementIntent")
bool FCombatMotionVoluntaryTranslationIntentTest::RunTest(
    const FString& Parameters)
{
    const UScriptStruct* Technique =
        FKashmirCombatTechniqueDefinition::StaticStruct();
    TestNotNull(TEXT("Body presentation remains explicit"),
        FindFProperty<FProperty>(Technique, TEXT("MovementIntent")));
    TestNotNull(TEXT("Voluntary translation remains explicit"),
        FindFProperty<FProperty>(Technique, TEXT("MovementSpec")));
    TestNotEqual(TEXT("Presentation and translation use distinct contracts"),
        StaticEnum<EKashmirMovementIntent>()->GetFName(),
        StaticEnum<EKashmirMovementDelivery>()->GetFName());
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionTranslationRotationTest,
    "VoluntaryTranslationIsSeparateFromRotation")
bool FCombatMotionTranslationRotationTest::RunTest(const FString& Parameters)
{
    const UScriptStruct* Spec = FKashmirTechniqueMovementSpec::StaticStruct();
    TestNull(TEXT("Translation spec has no rotation field"),
        FindFProperty<FProperty>(Spec, TEXT("Rotation")));
    TestNull(TEXT("Translation spec has no rotation delivery field"),
        FindFProperty<FProperty>(Spec, TEXT("RotationDelivery")));
    TestNull(TEXT("Technique has no decorative rotation spec yet"),
        FindFProperty<FProperty>(
            FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("RotationSpec")));
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionTranslationYawAuthorityTest,
    "TranslationDoesNotOwnYaw")
bool FCombatMotionTranslationYawAuthorityTest::RunTest(const FString& Parameters)
{
    const FString Runtime = LoadCombatMotionSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"));
    TestFalse(TEXT("Movement delivery source loads"), Runtime.IsEmpty());
    TestFalse(TEXT("Translation does not set actor rotation"),
        Runtime.Contains(TEXT("SetActorRotation")));
    TestFalse(TEXT("Translation does not add actor rotation"),
        Runtime.Contains(TEXT("AddActorWorldRotation")) ||
        Runtime.Contains(TEXT("AddActorLocalRotation")));
    TestFalse(TEXT("Translation does not set control rotation"),
        Runtime.Contains(TEXT("SetControlRotation")));
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionVoluntaryForcedBoundaryTest,
    "VoluntaryMovementIsSeparateFromForcedMovement")
bool FCombatMotionVoluntaryForcedBoundaryTest::RunTest(
    const FString& Parameters)
{
    TestNotNull(TEXT("Technique owns voluntary MovementSpec"),
        FindFProperty<FProperty>(
            FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementSpec")));
    TestNotNull(TEXT("Physical reaction exposes resolved direction"),
        FindFProperty<FProperty>(
            FKashmirPhysicalReactionResult::StaticStruct(), TEXT("Direction")));
    TestNotNull(TEXT("Physical reaction exposes resolved intensity"),
        FindFProperty<FProperty>(
            FKashmirPhysicalReactionResult::StaticStruct(), TEXT("Intensity")));
    TestNull(TEXT("Physical reaction does not reuse voluntary MovementSpec"),
        FindFProperty<FProperty>(
            FKashmirPhysicalReactionResult::StaticStruct(),
            TEXT("MovementSpec")));
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionEvasionBoundaryTest,
    "EvasionIsNotAMovementDeliveryMode")
bool FCombatMotionEvasionBoundaryTest::RunTest(const FString& Parameters)
{
    const UEnum* Delivery = StaticEnum<EKashmirMovementDelivery>();
    TestEqual(TEXT("Movement delivery remains primitive-only"),
        Delivery->NumEnums() - 1, 2);
    for (const TCHAR* Forbidden : {
        TEXT("Evasion"), TEXT("Dodge"), TEXT("Roll"),
        TEXT("Sidestep"), TEXT("PerfectDodge")})
    {
        TestEqual(*FString::Printf(TEXT("%s is not a delivery mode"), Forbidden),
            Delivery->GetIndexByNameString(Forbidden), INDEX_NONE);
    }
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionCastingBoundaryTest,
    "CastingIsNotAMovementDeliveryMode")
bool FCombatMotionCastingBoundaryTest::RunTest(const FString& Parameters)
{
    const UEnum* MovementDelivery = StaticEnum<EKashmirMovementDelivery>();
    for (const TCHAR* Forbidden : {
        TEXT("Cast"), TEXT("Projectile"), TEXT("Beam"), TEXT("Summon")})
    {
        TestEqual(*FString::Printf(TEXT("%s is not voluntary translation"),
            Forbidden),
            MovementDelivery->GetIndexByNameString(Forbidden), INDEX_NONE);
    }

    const UEnum* CombatDelivery = StaticEnum<EKashmirDeliveryType>();
    TestTrue(TEXT("Projectile already belongs to combat delivery"),
        CombatDelivery->GetIndexByNameString(TEXT("Projectile")) != INDEX_NONE);
    TestTrue(TEXT("Beam already belongs to combat delivery"),
        CombatDelivery->GetIndexByNameString(TEXT("Beam")) != INDEX_NONE);
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionRootMotionBoundaryTest,
    "RootMotionRemainsIndependent")
bool FCombatMotionRootMotionBoundaryTest::RunTest(const FString& Parameters)
{
    TestNull(TEXT("MovementSpec has no Root Motion field"),
        FindFProperty<FProperty>(FKashmirTechniqueMovementSpec::StaticStruct(),
            TEXT("RootMotion")));
    const FString Runtime = LoadCombatMotionSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"));
    TestFalse(TEXT("Movement delivery source loads"), Runtime.IsEmpty());
    TestFalse(TEXT("Movement delivery does not consume Root Motion"),
        Runtime.Contains(TEXT("RootMotion")));
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionStepForwardTranslationTest,
    "StepForwardStillUsesExistingTranslationGrammar")
bool FCombatMotionStepForwardTranslationTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Step =
        FindCombatMotionStepForward(LoadCombatMotionGrammarStyle());
    TestNotNull(TEXT("Persistent StepForward exists"), Step);
    if (Step == nullptr)
    {
        return false;
    }
    TestEqual(TEXT("StepForward remains FullBody presentation"),
        Step->MovementIntent, EKashmirMovementIntent::FullBody);
    TestEqual(TEXT("StepForward uses ControlledTranslation"),
        Step->MovementSpec.Delivery,
        EKashmirMovementDelivery::ControlledTranslation);
    TestEqual(TEXT("StepForward distance remains 80 cm"),
        Step->MovementSpec.Distance, 80.0f);
    TestEqual(TEXT("StepForward duration remains 0.25 s"),
        Step->MovementSpec.Duration, 0.25f);
    TestEqual(TEXT("StepForward remains Actor-relative"),
        Step->MovementSpec.Reference, EKashmirMovementReference::Actor);
    TestEqual(TEXT("StepForward remains Forward"),
        Step->MovementSpec.Direction, EKashmirMovementDirection::Forward);
    TestEqual(TEXT("StepForward remains target-independent"),
        Step->MovementSpec.TargetPolicy,
        EKashmirMovementTargetPolicy::NotRequired);
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionTargetRelativeGateTest,
    "TargetRelativeTranslationStillRejectedUntilExecutorExists")
bool FCombatMotionTargetRelativeGateTest::RunTest(const FString& Parameters)
{
    FKashmirTechniqueMovementSpec Spec;
    Spec.Delivery = EKashmirMovementDelivery::ControlledTranslation;
    Spec.Distance = 80.0f;
    Spec.Duration = 0.25f;
    Spec.Direction = EKashmirMovementDirection::Forward;
    Spec.Reference = EKashmirMovementReference::Target;
    Spec.TargetPolicy = EKashmirMovementTargetPolicy::Required;
    FString Reason;
    TestFalse(TEXT("Target-relative movement remains gated"),
        Spec.IsValid(Reason));
    TestTrue(TEXT("Gate explains the missing executor"),
        Reason.Contains(TEXT("not executable in v0.1")));
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionExistingAuthorityTest,
    "ExistingCombatContractsRemainAuthoritative")
bool FCombatMotionExistingAuthorityTest::RunTest(const FString& Parameters)
{
    const UScriptStruct* Technique =
        FKashmirCombatTechniqueDefinition::StaticStruct();
    for (const TCHAR* Property : {
        TEXT("ActionId"), TEXT("TechniqueFamily"), TEXT("AttackDirection"),
        TEXT("AttackShape"), TEXT("RuntimeDefinition"),
        TEXT("CombatDefinition"), TEXT("TechniqueTags")})
    {
        TestNotNull(*FString::Printf(TEXT("Technique retains %s"), Property),
            FindFProperty<FProperty>(Technique, Property));
    }

    const UEnum* Delivery = StaticEnum<EKashmirDeliveryType>();
    for (const TCHAR* Existing : {
        TEXT("Contact"), TEXT("Projectile"), TEXT("Beam"), TEXT("Area"),
        TEXT("Field"), TEXT("Grapple"), TEXT("Self"), TEXT("Target")})
    {
        TestTrue(*FString::Printf(TEXT("Combat delivery retains %s"), Existing),
            Delivery->GetIndexByNameString(Existing) != INDEX_NONE);
    }
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionGenericRuntimeTest,
    "NoTechniqueIdSpecificRuntimeBranching")
bool FCombatMotionGenericRuntimeTest::RunTest(const FString& Parameters)
{
    const TArray<FString> Sources = {
        LoadCombatMotionSource(
            TEXT("Source/KashmirUE/Private/Combat/KashmirCombatTechnique.cpp")),
        LoadCombatMotionSource(
            TEXT("Source/KashmirUE/Private/Combat/KashmirDirectionalSwordComponent.cpp")),
        LoadCombatMotionSource(
            TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"))};
    for (const FString& Source : Sources)
    {
        TestFalse(TEXT("Runtime source was loaded"), Source.IsEmpty());
        TestFalse(TEXT("Runtime does not branch on StepForward identity"),
            Source.Contains(CombatMotionGrammarStepForwardId.ToString()));
    }
    return true;
}


KASHMIR_COMBAT_MOTION_GRAMMAR_TEST(
    FCombatMotionNoMegaEnumTest,
    "NoCombatMotionMegaEnum")
bool FCombatMotionNoMegaEnumTest::RunTest(const FString& Parameters)
{
    const UEnum* Delivery = StaticEnum<EKashmirMovementDelivery>();
    for (const TCHAR* Forbidden : {
        TEXT("Lunge"), TEXT("Pivot"), TEXT("Dodge"), TEXT("Parry"),
        TEXT("Cast"), TEXT("Grapple"), TEXT("Knockback"), TEXT("Launch")})
    {
        TestEqual(*FString::Printf(TEXT("%s is not a movement primitive"),
            Forbidden),
            Delivery->GetIndexByNameString(Forbidden), INDEX_NONE);
    }
    TestNull(TEXT("Technique has no CombatMotionSemantic field"),
        FindFProperty<FProperty>(
            FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("CombatMotionSemantic")));
    TestNull(TEXT("Technique has no MovementSemantic field"),
        FindFProperty<FProperty>(
            FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementSemantic")));
    return true;
}


#endif
