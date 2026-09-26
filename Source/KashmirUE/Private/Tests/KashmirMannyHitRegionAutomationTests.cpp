#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirHitEvidenceBuilder.h"
#include "Combat/KashmirHitRegionMap.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMannyHitRegionPhysicsTest,
    "Kashmir.Combat.Anatomy.MannyPhysicsAsset",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirMannyHitRegionPhysicsTest::RunTest(
    const FString& Parameters)
{
    const USkeletalMesh* MannyMesh =
        LoadObject<USkeletalMesh>(
            nullptr,
            TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")
        );

    if (!TestNotNull(
            TEXT("Manny skeletal mesh loads"),
            MannyMesh))
    {
        return false;
    }

    UPhysicsAsset* PhysicsAsset =
        MannyMesh->GetPhysicsAsset();

    if (!TestNotNull(
            TEXT("Manny has a physics asset"),
            PhysicsAsset))
    {
        return false;
    }

    TestTrue(
        TEXT("Manny physics asset contains bodies"),
        PhysicsAsset->SkeletalBodySetups.Num() > 0
    );

    bool bFoundHead = false;
    bool bFoundTorso = false;
    bool bFoundPelvis = false;
    bool bFoundLeftArm = false;
    bool bFoundRightArm = false;
    bool bFoundLeftLeg = false;
    bool bFoundRightLeg = false;

    for (const USkeletalBodySetup* BodySetup :
        PhysicsAsset->SkeletalBodySetups)
    {
        if (BodySetup == nullptr)
        {
            continue;
        }

        const FName BoneName =
            BodySetup->BoneName;

        if (BoneName == TEXT("head"))
        {
            bFoundHead = true;
        }
        else if (
            BoneName == TEXT("spine_02") ||
            BoneName == TEXT("spine_03") ||
            BoneName == TEXT("spine_04") ||
            BoneName == TEXT("spine_05"))
        {
            bFoundTorso = true;
        }
        else if (BoneName == TEXT("pelvis"))
        {
            bFoundPelvis = true;
        }
        else if (
            BoneName == TEXT("upperarm_l") ||
            BoneName == TEXT("lowerarm_l") ||
            BoneName == TEXT("hand_l"))
        {
            bFoundLeftArm = true;
        }
        else if (
            BoneName == TEXT("upperarm_r") ||
            BoneName == TEXT("lowerarm_r") ||
            BoneName == TEXT("hand_r"))
        {
            bFoundRightArm = true;
        }
        else if (
            BoneName == TEXT("thigh_l") ||
            BoneName == TEXT("calf_l") ||
            BoneName == TEXT("foot_l"))
        {
            bFoundLeftLeg = true;
        }
        else if (
            BoneName == TEXT("thigh_r") ||
            BoneName == TEXT("calf_r") ||
            BoneName == TEXT("foot_r"))
        {
            bFoundRightLeg = true;
        }
    }

    TestTrue(
        TEXT("Physics asset covers head"),
        bFoundHead
    );

    TestTrue(
        TEXT("Physics asset covers torso"),
        bFoundTorso
    );

    TestTrue(
        TEXT("Physics asset covers pelvis"),
        bFoundPelvis
    );

    TestTrue(
        TEXT("Physics asset covers left arm"),
        bFoundLeftArm
    );

    TestTrue(
        TEXT("Physics asset covers right arm"),
        bFoundRightArm
    );

    TestTrue(
        TEXT("Physics asset covers left leg"),
        bFoundLeftLeg
    );

    TestTrue(
        TEXT("Physics asset covers right leg"),
        bFoundRightLeg
    );

    return true;
}

#endif