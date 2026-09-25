#include "Misc/AutomationTest.h"

#include "Combat/KashmirHitRegionMap.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHitRegionResolveTest,
    "Kashmir.Combat.HitRegion.Resolve",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHitRegionResolveTest::RunTest(
    const FString& Parameters)
{
    UKashmirHitRegionMap* Map =
        NewObject<UKashmirHitRegionMap>();

    TestNotNull(
        TEXT("Hit region map can be created"),
        Map
    );

    if (Map == nullptr)
    {
        return false;
    }

    const FGameplayTag TorsoTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Torso")
        );

    const FGameplayTag LeftLegTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Leg.Left")
        );

    Map->BoneToRegion.Add(
        TEXT("spine_03"),
        TorsoTag
    );

    Map->BoneToRegion.Add(
        TEXT("thigh_l"),
        LeftLegTag
    );

    FGameplayTag Region;

    TestTrue(
        TEXT("Torso bone resolves"),
        Map->ResolveRegion(
            TEXT("spine_03"),
            Region
        )
    );

    TestTrue(
        TEXT("Torso region is correct"),
        Region.MatchesTagExact(
            TorsoTag
        )
    );

    TestTrue(
        TEXT("Left leg bone resolves"),
        Map->ResolveRegion(
            TEXT("thigh_l"),
            Region
        )
    );

    TestTrue(
        TEXT("Left leg region is correct"),
        Region.MatchesTagExact(
            LeftLegTag
        )
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHitRegionUnknownBoneTest,
    "Kashmir.Combat.HitRegion.UnknownBone",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHitRegionUnknownBoneTest::RunTest(
    const FString& Parameters)
{
    UKashmirHitRegionMap* Map =
        NewObject<UKashmirHitRegionMap>();

    TestNotNull(
        TEXT("Hit region map can be created"),
        Map
    );

    if (Map == nullptr)
    {
        return false;
    }

    FGameplayTag Region;

    TestFalse(
        TEXT("Unknown bone does not resolve"),
        Map->ResolveRegion(
            TEXT("unknown_bone"),
            Region
        )
    );

    TestFalse(
        TEXT("Unknown bone produces no region"),
        Region.IsValid()
    );

    return true;
}

#endif