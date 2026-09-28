#if WITH_DEV_AUTOMATION_TESTS

#include "KashmirAnimInstance.h"
#include "KashmirCharacter.h"
#include "Combat/KashmirSwordPresentationComponent.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordAnimBridgeContractTest,
    "Kashmir.Animation.DirectionalSword.BridgeContract",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordAnimBridgeContractTest::RunTest(const FString& Parameters)
{
    const AKashmirCharacter* CharacterDefaults =
        GetDefault<AKashmirCharacter>();
    TestNotNull(TEXT("Character owns sword presentation component"),
        CharacterDefaults->GetSwordPresentationComponent());
    TestNotNull(TEXT("Character owns directional sword runtime component"),
        CharacterDefaults->GetDirectionalSwordComponent());

    const UKashmirAnimInstance* AnimInstance =
        GetDefault<UKashmirAnimInstance>();
    TestFalse(TEXT("Animation bridge defaults to disabled pose"),
        AnimInstance->GetSwordPose().bEnabled);

    const FStructProperty* PoseProperty = FindFProperty<FStructProperty>(
        UKashmirAnimInstance::StaticClass(),
        TEXT("SwordPose"));
    TestNotNull(TEXT("Sword pose snapshot is reflected"), PoseProperty);
    if (PoseProperty != nullptr)
    {
        TestTrue(TEXT("Sword pose snapshot is Blueprint-readable"),
            PoseProperty->HasAnyPropertyFlags(CPF_BlueprintVisible));
        TestTrue(TEXT("Sword pose snapshot preserves contract type"),
            PoseProperty->Struct.Get() ==
                FKashmirSwordPoseResult::StaticStruct());
    }

    const FStructProperty* RigCurvesProperty = FindFProperty<FStructProperty>(
        UKashmirAnimInstance::StaticClass(),
        TEXT("SwordRigCurves"));
    TestNotNull(TEXT("Sword rig curve bridge is reflected"), RigCurvesProperty);
    if (RigCurvesProperty != nullptr)
    {
        TestTrue(TEXT("Sword rig curve bridge is Blueprint-readable"),
            RigCurvesProperty->HasAnyPropertyFlags(CPF_BlueprintVisible));
        TestTrue(TEXT("Sword rig curve bridge preserves contract type"),
            RigCurvesProperty->Struct.Get() ==
                FKashmirSwordRigCurveValues::StaticStruct());
    }

    return true;
}

#endif
