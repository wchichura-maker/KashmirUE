#if WITH_DEV_AUTOMATION_TESTS

#include "KashmirAnimInstance.h"
#include "KashmirCharacter.h"
#include "Combat/KashmirSwordPresentationComponent.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirSwordRigAdapter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

namespace
{
    constexpr const TCHAR* SwordRigInputProperties[] =
    {
        TEXT("SwordLeadHandOffsetX"), TEXT("SwordLeadHandOffsetY"), TEXT("SwordLeadHandOffsetZ"),
        TEXT("SwordSupportHandOffsetX"), TEXT("SwordSupportHandOffsetY"), TEXT("SwordSupportHandOffsetZ"),
        TEXT("SwordAimPitch"), TEXT("SwordAimYaw"), TEXT("SwordAimRoll"),
        TEXT("SwordBodyLean"), TEXT("SwordPoseAlpha"),
        TEXT("SwordLeftFootLockAlpha"), TEXT("SwordRightFootLockAlpha")
    };
}


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

    for (const TCHAR* PropertyName : SwordRigInputProperties)
    {
        const FFloatProperty* Property = FindFProperty<FFloatProperty>(
            UKashmirAnimInstance::StaticClass(), PropertyName);
        TestNotNull(FString::Printf(TEXT("%s input is reflected"), PropertyName), Property);
        if (Property != nullptr)
        {
            TestTrue(FString::Printf(TEXT("%s is Blueprint-readable"), PropertyName),
                Property->HasAnyPropertyFlags(CPF_BlueprintVisible));
            TestTrue(FString::Printf(TEXT("%s is a transient snapshot"), PropertyName),
                Property->HasAnyPropertyFlags(CPF_Transient));
        }
    }

    TestNull(TEXT("Animation bridge no longer exposes a curve transport struct"),
        FindFProperty<FProperty>(UKashmirAnimInstance::StaticClass(), TEXT("SwordRigCurves")));

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigInputBridgeSourcePoseTest,
    "Kashmir.Animation.DirectionalSword.SwordRigInputBridge.SourcePose",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigInputBridgeSourcePoseTest::RunTest(const FString& Parameters)
{
    FKashmirSwordPoseResult Pose;
    Pose.bEnabled = true;
    Pose.PhaseAlpha = 0.5f;
    Pose.LeadHandOffset = FVector(4.0f, 3.0f, 2.0f);
    Pose.SupportHandOffset = FVector(1.0f, -2.0f, 3.0f);
    Pose.AimRotation = FRotator(-10.0f, 20.0f, 5.0f);
    Pose.BodyLeanDegrees = 7.0f;
    Pose.LeftFootLockAlpha = 0.75f;
    Pose.RightFootLockAlpha = 0.5f;

    FKashmirSwordRigInputs Inputs;
    FString Reason;
    FKashmirSwordRigAdapter Adapter;
    TestTrue(TEXT("Semantic source pose resolves into the direct input contract"),
        Adapter.Resolve(Pose, Inputs, Reason));
    TestEqual(TEXT("Source lead-hand value survives resolution"),
        Inputs.LeadHandOffset, Pose.LeadHandOffset);
    TestEqual(TEXT("Source aim value survives resolution"),
        Inputs.AimRotation, Pose.AimRotation);
    TestEqual(TEXT("Enabled source pose activates the rig"),
        Inputs.SwordPoseAlpha, 1.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigInputBridgeNonZeroValuesTest,
    "Kashmir.Animation.DirectionalSword.SwordRigInputBridge.NonZeroValues",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigInputBridgeNonZeroValuesTest::RunTest(const FString& Parameters)
{
    USkeletalMeshComponent* Mesh = NewObject<USkeletalMeshComponent>();
    UKashmirAnimInstance* AnimInstance = NewObject<UKashmirAnimInstance>(Mesh);
    float ExpectedValue = 1.0f;
    for (const TCHAR* PropertyName : SwordRigInputProperties)
    {
        FFloatProperty* Property = FindFProperty<FFloatProperty>(
            UKashmirAnimInstance::StaticClass(), PropertyName);
        TestNotNull(FString::Printf(TEXT("%s exists"), PropertyName), Property);
        if (Property != nullptr)
        {
            Property->SetPropertyValue_InContainer(AnimInstance, ExpectedValue);
            TestEqual(FString::Printf(TEXT("%s preserves a non-zero snapshot"), PropertyName),
                Property->GetPropertyValue_InContainer(AnimInstance), ExpectedValue);
        }
        ExpectedValue += 1.0f;
    }

    const FKashmirSwordRigInputs Inputs = AnimInstance->GetSwordRigInputs();
    TestEqual(TEXT("Snapshot reconstructs lead-hand X"), Inputs.LeadHandOffset.X, 1.0);
    TestEqual(TEXT("Snapshot reconstructs support-hand X"), Inputs.SupportHandOffset.X, 4.0);
    TestEqual(TEXT("Snapshot reconstructs pose alpha"), Inputs.SwordPoseAlpha, 11.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigInputBridgeZeroPoseTest,
    "Kashmir.Animation.DirectionalSword.SwordRigInputBridge.ZeroPose",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigInputBridgeZeroPoseTest::RunTest(const FString& Parameters)
{
    const UKashmirAnimInstance* AnimInstance = GetDefault<UKashmirAnimInstance>();
    const FKashmirSwordRigInputs Inputs = AnimInstance->GetSwordRigInputs();
    TestEqual(TEXT("Neutral pose has zero lead hand"), Inputs.LeadHandOffset, FVector::ZeroVector);
    TestEqual(TEXT("Neutral pose has zero support hand"), Inputs.SupportHandOffset, FVector::ZeroVector);
    TestEqual(TEXT("Neutral pose has zero aim"), Inputs.AimRotation, FRotator::ZeroRotator);
    TestEqual(TEXT("Neutral pose disables rig"), Inputs.SwordPoseAlpha, 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigInputBridgeNoCurveDependencyTest,
    "Kashmir.Animation.DirectionalSword.SwordRigInputBridge.NoAnimationCurveDependency",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigInputBridgeNoCurveDependencyTest::RunTest(const FString& Parameters)
{
    TestNull(TEXT("Legacy curve struct is absent"),
        FindFProperty<FProperty>(UKashmirAnimInstance::StaticClass(), TEXT("SwordRigCurves")));
    TestNull(TEXT("Legacy curve publisher is absent"),
        UKashmirAnimInstance::StaticClass()->FindFunctionByName(TEXT("PublishSwordRigCurves")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirSwordRigInputBridgeNeutralOverrideTest,
    "Kashmir.Animation.DirectionalSword.SwordRigInputBridge.NeutralOverride",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirSwordRigInputBridgeNeutralOverrideTest::RunTest(const FString& Parameters)
{
    USkeletalMeshComponent* Mesh = NewObject<USkeletalMeshComponent>();
    UKashmirAnimInstance* AnimInstance = NewObject<UKashmirAnimInstance>(Mesh);

    FKashmirSwordPoseResult SourcePose;
    SourcePose.bEnabled = true;
    SourcePose.PhaseAlpha = 0.6f;
    SourcePose.LeadHandOffset = FVector(7.0f, 6.0f, 5.0f);
    SourcePose.SupportHandOffset = FVector(4.0f, 3.0f, 2.0f);
    SourcePose.AimRotation = FRotator(11.0f, 22.0f, 33.0f);
    SourcePose.BodyLeanDegrees = 9.0f;
    SourcePose.LeftFootLockAlpha = 0.75f;
    SourcePose.RightFootLockAlpha = 0.5f;

    FStructProperty* PoseProperty = FindFProperty<FStructProperty>(
        UKashmirAnimInstance::StaticClass(), TEXT("SwordPose"));
    TestNotNull(TEXT("SwordPose test source is reflected"), PoseProperty);
    if (PoseProperty == nullptr)
    {
        return false;
    }
    PoseProperty->Struct->CopyScriptStruct(
        PoseProperty->ContainerPtrToValuePtr<void>(AnimInstance), &SourcePose);

    AnimInstance->SetForceNeutralSwordRigInputs(false);
    const FKashmirSwordRigInputs NormalBefore = AnimInstance->GetSwordRigInputs();
    TestFalse(TEXT("Override starts disabled"),
        AnimInstance->IsForceNeutralSwordRigInputsEnabled());
    TestEqual(TEXT("OFF preserves non-zero hand input"),
        NormalBefore.LeadHandOffset, SourcePose.LeadHandOffset);
    TestEqual(TEXT("OFF activates the rig"), NormalBefore.SwordPoseAlpha, 1.0f);

    AnimInstance->SetForceNeutralSwordRigInputs(true);
    const FKashmirSwordRigInputs Neutral = AnimInstance->GetSwordRigInputs();
    TestTrue(TEXT("Override reports enabled"),
        AnimInstance->IsForceNeutralSwordRigInputsEnabled());
    TestEqual(TEXT("ON neutralizes lead hand"), Neutral.LeadHandOffset, FVector::ZeroVector);
    TestEqual(TEXT("ON neutralizes support hand"), Neutral.SupportHandOffset, FVector::ZeroVector);
    TestEqual(TEXT("ON neutralizes aim"), Neutral.AimRotation, FRotator::ZeroRotator);
    TestEqual(TEXT("ON neutralizes body lean"), Neutral.BodyLeanDegrees, 0.0f);
    TestEqual(TEXT("ON neutralizes pose alpha"), Neutral.SwordPoseAlpha, 0.0f);
    TestEqual(TEXT("ON neutralizes left foot"), Neutral.LeftFootLockAlpha, 0.0f);
    TestEqual(TEXT("ON neutralizes right foot"), Neutral.RightFootLockAlpha, 0.0f);

    const FKashmirSwordPoseResult PreservedPose = AnimInstance->GetSwordPose();
    TestTrue(TEXT("Neutral override preserves source-pose enabled state"), PreservedPose.bEnabled);
    TestEqual(TEXT("Neutral override preserves source-pose hand data"),
        PreservedPose.LeadHandOffset, SourcePose.LeadHandOffset);
    TestEqual(TEXT("Neutral override preserves source-pose aim data"),
        PreservedPose.AimRotation, SourcePose.AimRotation);

    AnimInstance->SetForceNeutralSwordRigInputs(false);
    const FKashmirSwordRigInputs NormalAfter = AnimInstance->GetSwordRigInputs();
    TestFalse(TEXT("Override returns to disabled"),
        AnimInstance->IsForceNeutralSwordRigInputsEnabled());
    TestEqual(TEXT("OFF again restores hand input"),
        NormalAfter.LeadHandOffset, SourcePose.LeadHandOffset);
    TestEqual(TEXT("OFF again restores aim input"),
        NormalAfter.AimRotation, SourcePose.AimRotation);
    TestEqual(TEXT("OFF again restores pose alpha"), NormalAfter.SwordPoseAlpha, 1.0f);
    return true;
}

#endif
