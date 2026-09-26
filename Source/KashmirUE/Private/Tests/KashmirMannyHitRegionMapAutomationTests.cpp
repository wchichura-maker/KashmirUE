#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirHitRegionMap.h"

#include "Engine/SkeletalMesh.h"
#include "Misc/AutomationTest.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMannyHitRegionMapTest,
    "Kashmir.Combat.Anatomy.MannyHitRegionMap",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirMannyHitRegionMapTest::RunTest(
    const FString& Parameters)
{
    const USkeletalMesh* MannyMesh =
        LoadObject<USkeletalMesh>(
            nullptr,
            TEXT(
                "/Game/Characters/Mannequins/Meshes/"
                "SKM_Manny_Simple.SKM_Manny_Simple"
            )
        );

    if (!TestNotNull(
            TEXT("Manny skeletal mesh loads"),
            MannyMesh))
    {
        return false;
    }

    const UPhysicsAsset* PhysicsAsset =
        MannyMesh->GetPhysicsAsset();

    if (!TestNotNull(
            TEXT("Manny physics asset exists"),
            PhysicsAsset))
    {
        return false;
    }

    const UKashmirHitRegionMap* HitRegionMap =
        LoadObject<UKashmirHitRegionMap>(
            nullptr,
            TEXT(
                "/Game/KashmirAct/Combat/HitRegions/"
                "DA_HitRegion_Manny.DA_HitRegion_Manny"
            )
        );

    if (!TestNotNull(
            TEXT("Manny hit region map loads"),
            HitRegionMap))
    {
        return false;
    }

    int32 TestedBodies = 0;
    int32 ResolvedBodies = 0;

    for (const USkeletalBodySetup* BodySetup :
        PhysicsAsset->SkeletalBodySetups)
    {
        if (BodySetup == nullptr)
        {
            continue;
        }

        const FName BoneName =
            BodySetup->BoneName;

        if (BoneName.IsNone())
        {
            continue;
        }

        ++TestedBodies;

        FGameplayTag Region;

        const bool bResolved =
            HitRegionMap->ResolveRegion(
                BoneName,
                Region
            );

        if (bResolved)
        {
            ++ResolvedBodies;

            AddInfo(
                FString::Printf(
                    TEXT(
                        "Bone '%s' -> '%s'"
                    ),
                    *BoneName.ToString(),
                    *Region.ToString()
                )
            );
        }
        else
        {
            AddWarning(
                FString::Printf(
                    TEXT(
                        "Physics body bone '%s' has no semantic region"
                    ),
                    *BoneName.ToString()
                )
            );
        }
    }

    TestTrue(
        TEXT(
            "Physics asset exposes bodies for mapping"
        ),
        TestedBodies > 0
    );

    TestEqual(
        TEXT(
            "Every Manny physics body has a semantic hit region"
        ),
        ResolvedBodies,
        TestedBodies
    );

    return true;
}

#endif