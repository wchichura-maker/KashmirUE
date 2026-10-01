#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatTechnique.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"


namespace
{
    constexpr const TCHAR* OffensiveMovementStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    const FName StepForwardTechniqueId(TEXT("Technique.Sword.Test.StepForward"));

    const UKashmirWeaponCombatStyle* LoadOffensiveMovementStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(
            nullptr, OffensiveMovementStylePath);
    }

    const FKashmirCombatTechniqueDefinition* FindStepForward(
        const UKashmirWeaponCombatStyle* Style)
    {
        return Style != nullptr
            ? Style->Techniques.FindByPredicate(
                [](const FKashmirCombatTechniqueDefinition& Technique)
                {
                    return Technique.TechniqueId == StepForwardTechniqueId;
                })
            : nullptr;
    }

    FString LoadRuntimeSource(const TCHAR* RelativePath)
    {
        FString Source;
        FFileHelper::LoadFileToString(
            Source,
            *FPaths::Combine(FPaths::ProjectDir(), RelativePath));
        return Source;
    }
}


#define KASHMIR_OFFENSIVE_MOVEMENT_TEST(ClassName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
        ClassName, \
        "Kashmir.Combat.OffensiveMovementGrammar." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementStepForwardTest,
    "StepForwardRemainsValid")
bool FOffensiveMovementStepForwardTest::RunTest(const FString& Parameters)
{
    const FKashmirCombatTechniqueDefinition* Step =
        FindStepForward(LoadOffensiveMovementStyle());
    TestNotNull(TEXT("Persistent StepForward exists"), Step);
    if (Step == nullptr) return false;
    FString Reason;
    TestTrue(TEXT("StepForward remains valid"), Step->IsValid(Reason));
    TestEqual(TEXT("StepForward remains Actor-relative"),
        Step->MovementSpec.Reference, EKashmirMovementReference::Actor);
    TestEqual(TEXT("StepForward remains Forward"),
        Step->MovementSpec.Direction, EKashmirMovementDirection::Forward);
    TestEqual(TEXT("StepForward does not require a target"),
        Step->MovementSpec.TargetPolicy,
        EKashmirMovementTargetPolicy::NotRequired);
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementOrthogonalityTest,
    "IntentDeliveryReferenceAreOrthogonal")
bool FOffensiveMovementOrthogonalityTest::RunTest(const FString& Parameters)
{
    TestNotNull(TEXT("MovementIntent remains on Technique"),
        FindFProperty<FProperty>(FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementIntent")));
    TestNotNull(TEXT("MovementSpec remains on Technique"),
        FindFProperty<FProperty>(FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementSpec")));
    TestNotNull(TEXT("Direction is explicit in MovementSpec"),
        FindFProperty<FProperty>(FKashmirTechniqueMovementSpec::StaticStruct(),
            TEXT("Direction")));
    TestNotNull(TEXT("Reference is separate from Direction"),
        FindFProperty<FProperty>(FKashmirTechniqueMovementSpec::StaticStruct(),
            TEXT("Reference")));
    TestNotEqual(TEXT("Intent and Delivery use distinct enum types"),
        StaticEnum<EKashmirMovementIntent>()->GetFName(),
        StaticEnum<EKashmirMovementDelivery>()->GetFName());
    TestNotEqual(TEXT("Direction and Reference use distinct enum types"),
        StaticEnum<EKashmirMovementDirection>()->GetFName(),
        StaticEnum<EKashmirMovementReference>()->GetFName());
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementDirectionFrameTest,
    "DirectionAndReferenceCompose")
bool FOffensiveMovementDirectionFrameTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Direction has four cardinal values"),
        StaticEnum<EKashmirMovementDirection>()->NumEnums() - 1, 4);
    TestEqual(TEXT("Reference has Actor and Target"),
        StaticEnum<EKashmirMovementReference>()->NumEnums() - 1, 2);
    TestEqual(TEXT("Target policy is explicit"),
        StaticEnum<EKashmirMovementTargetPolicy>()->NumEnums() - 1, 3);

    FKashmirTechniqueMovementSpec TargetOrbitCandidate;
    TargetOrbitCandidate.Reference = EKashmirMovementReference::Target;
    TargetOrbitCandidate.Direction = EKashmirMovementDirection::Left;
    TargetOrbitCandidate.TargetPolicy = EKashmirMovementTargetPolicy::Required;
    TestEqual(TEXT("Target and Left remain independent fields"),
        TargetOrbitCandidate.Reference, EKashmirMovementReference::Target);
    TestEqual(TEXT("Tangential direction remains data"),
        TargetOrbitCandidate.Direction, EKashmirMovementDirection::Left);
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementTargetPolicyTest,
    "TargetRelativeIsRepresentedButNotExecutable")
bool FOffensiveMovementTargetPolicyTest::RunTest(const FString& Parameters)
{
    FKashmirTechniqueMovementSpec Spec;
    Spec.Delivery = EKashmirMovementDelivery::ControlledTranslation;
    Spec.Distance = 100.0f;
    Spec.Duration = 0.25f;
    Spec.Direction = EKashmirMovementDirection::Forward;
    Spec.Reference = EKashmirMovementReference::Target;
    Spec.TargetPolicy = EKashmirMovementTargetPolicy::Required;
    FString Reason;
    TestFalse(TEXT("Target-relative execution is rejected in v0.1"),
        Spec.IsValid(Reason));
    TestTrue(TEXT("Rejection identifies the execution boundary"),
        Reason.Contains(TEXT("not executable in v0.1")));

    Spec.TargetPolicy = EKashmirMovementTargetPolicy::NotRequired;
    TestFalse(TEXT("Target reference cannot omit target policy"),
        Spec.IsValid(Reason));
    TestTrue(TEXT("Missing target policy is explicit"),
        Reason.Contains(TEXT("explicit target policy")));
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementDirectionGateTest,
    "FutureDirectionsAreSafe")
bool FOffensiveMovementDirectionGateTest::RunTest(const FString& Parameters)
{
    FKashmirTechniqueMovementSpec Spec;
    Spec.Delivery = EKashmirMovementDelivery::ControlledTranslation;
    Spec.Distance = 80.0f;
    Spec.Duration = 0.25f;
    Spec.Reference = EKashmirMovementReference::Actor;
    Spec.Direction = EKashmirMovementDirection::Backward;
    FString Reason;
    TestFalse(TEXT("Backward is not silently executed as Forward"),
        Spec.IsValid(Reason));
    TestTrue(TEXT("Future direction gate is observable"),
        Reason.Contains(TEXT("not executable in v0.1")));
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementExistingTechniquesTest,
    "ExistingTechniquesRemainValid")
bool FOffensiveMovementExistingTechniquesTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadOffensiveMovementStyle();
    TestNotNull(TEXT("Baseline Sword style loads"), Style);
    if (Style == nullptr) return false;
    for (const FKashmirCombatTechniqueDefinition& Technique : Style->Techniques)
    {
        FString Reason;
        const bool bIsValid = Technique.IsValid(Reason);
        TestTrue(*FString::Printf(TEXT("%s remains valid: %s"),
            *Technique.TechniqueId.ToString(), *Reason), bIsValid);
    }
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementRuntimeGenericTest,
    "NoTechniqueSpecificRuntimeBranching")
bool FOffensiveMovementRuntimeGenericTest::RunTest(const FString& Parameters)
{
    const TArray<FString> Sources = {
        LoadRuntimeSource(TEXT("Source/KashmirUE/Private/Combat/KashmirCombatTechnique.cpp")),
        LoadRuntimeSource(TEXT("Source/KashmirUE/Private/Combat/KashmirDirectionalSwordComponent.cpp")),
        LoadRuntimeSource(TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"))};
    for (const FString& Source : Sources)
    {
        TestFalse(TEXT("Runtime source was loaded"), Source.IsEmpty());
        TestFalse(TEXT("Runtime does not branch on StepForward id"),
            Source.Contains(StepForwardTechniqueId.ToString()));
    }
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementAuthorityTest,
    "TranslationRotationAndRootMotionStaySeparate")
bool FOffensiveMovementAuthorityTest::RunTest(const FString& Parameters)
{
    const UScriptStruct* SpecStruct = FKashmirTechniqueMovementSpec::StaticStruct();
    TestNull(TEXT("MovementSpec does not claim rotation authority"),
        FindFProperty<FProperty>(SpecStruct, TEXT("Rotation")));
    TestNull(TEXT("MovementSpec does not claim Root Motion authority"),
        FindFProperty<FProperty>(SpecStruct, TEXT("RootMotion")));

    const FString Runtime = LoadRuntimeSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"));
    TestFalse(TEXT("Translation runtime does not set actor rotation"),
        Runtime.Contains(TEXT("SetActorRotation")));
    TestFalse(TEXT("Translation runtime does not consume Root Motion"),
        Runtime.Contains(TEXT("RootMotion")));
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementPivotBoundaryTest,
    "PivotIsNotImplicitTranslation")
bool FOffensiveMovementPivotBoundaryTest::RunTest(const FString& Parameters)
{
    const UEnum* Delivery = StaticEnum<EKashmirMovementDelivery>();
    TestEqual(TEXT("Delivery remains primitive-only"), Delivery->NumEnums() - 1, 2);
    TestEqual(TEXT("No Pivot delivery exists"),
        Delivery->GetIndexByNameString(TEXT("Pivot")), INDEX_NONE);
    TestNull(TEXT("No implicit RotationDelivery is authored in translation spec"),
        FindFProperty<FProperty>(FKashmirTechniqueMovementSpec::StaticStruct(),
            TEXT("RotationDelivery")));
    return true;
}


KASHMIR_OFFENSIVE_MOVEMENT_TEST(FOffensiveMovementNoSemanticEnumTest,
    "NoDecorativeSemanticTaxonomy")
bool FOffensiveMovementNoSemanticEnumTest::RunTest(const FString& Parameters)
{
    const UEnum* Delivery = StaticEnum<EKashmirMovementDelivery>();
    for (const TCHAR* Forbidden : {
        TEXT("StepForward"), TEXT("Lunge"), TEXT("Advance"),
        TEXT("Retreat"), TEXT("Pivot")})
    {
        TestEqual(*FString::Printf(TEXT("%s is not a delivery primitive"), Forbidden),
            Delivery->GetIndexByNameString(Forbidden), INDEX_NONE);
    }
    TestNull(TEXT("No decorative MovementSemantic property exists"),
        FindFProperty<FProperty>(FKashmirCombatTechniqueDefinition::StaticStruct(),
            TEXT("MovementSemantic")));
    return true;
}


#endif
