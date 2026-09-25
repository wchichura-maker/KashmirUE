#include "Misc/AutomationTest.h"

#include "Combat/KashmirHitEvidenceBuilder.h"
#include "Combat/KashmirHitRegionMap.h"
#include "Engine/HitResult.h"

#if WITH_DEV_AUTOMATION_TESTS


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHitEvidenceBuildTest,
    "Kashmir.Combat.HitEvidence.BuildFromHitResult",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHitEvidenceBuildTest::RunTest(
    const FString& Parameters)
{
    FHitResult Hit;
    Hit.bBlockingHit = true;

    Hit.ImpactPoint =
        FVector(100.0, 50.0, 120.0);

    Hit.ImpactNormal =
        FVector(-1.0, 0.0, 0.0);

    Hit.BoneName =
        TEXT("spine_03");

    FKashmirHitEvidenceBuildInput Input;

    Input.InstigatorId =
        TEXT("Player");

    Input.TargetId =
        TEXT("Enemy_01");

    Input.ContactSource =
        EKashmirContactSourceType::Weapon;

    Input.SourceId =
        TEXT("Sword_01");

    Input.ContactVelocity =
        FVector(800.0, 0.0, 0.0);

    Input.AttackDirection =
        FVector(1.0, 0.0, 0.0);

    Input.RelativeSpeed =
        800.0f;

    UKashmirHitRegionMap* RegionMap =
        NewObject<UKashmirHitRegionMap>();

    const FGameplayTag TorsoTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Torso")
        );

    RegionMap->BoneToRegion.Add(
        TEXT("spine_03"),
        TorsoTag
    );

    FKashmirHitEvidence Evidence;
    FString Reason;

    const bool bBuilt =
        FKashmirHitEvidenceBuilder::BuildFromHitResult(
            Hit,
            Input,
            RegionMap,
            Evidence,
            Reason
        );

    TestTrue(
        TEXT("Blocking hit builds evidence"),
        bBuilt
    );

    TestEqual(
        TEXT("Instigator id is preserved"),
        Evidence.InstigatorId,
        FName(TEXT("Player"))
    );

    TestEqual(
        TEXT("Target id is preserved"),
        Evidence.TargetId,
        FName(TEXT("Enemy_01"))
    );

    TestEqual(
        TEXT("Contact source is weapon"),
        Evidence.ContactSource,
        EKashmirContactSourceType::Weapon
    );

    TestEqual(
        TEXT("Source id is preserved"),
        Evidence.SourceId,
        FName(TEXT("Sword_01"))
    );

    TestEqual(
        TEXT("Impact point comes from hit result"),
        Evidence.ImpactPoint,
        FVector(100.0, 50.0, 120.0)
    );

    TestEqual(
        TEXT("Hit bone comes from hit result"),
        Evidence.HitBone,
        FName(TEXT("spine_03"))
    );

    TestTrue(
        TEXT("Bone resolves semantic torso region"),
        Evidence.HitRegion.MatchesTagExact(
            TorsoTag
        )
    );

    TestTrue(
        TEXT("Attack direction is normalized"),
        Evidence.AttackDirection.Equals(
            FVector(1.0, 0.0, 0.0)
        )
    );

    TestTrue(
        TEXT("Build succeeds without error"),
        Reason.IsEmpty()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHitEvidenceRejectNonBlockingTest,
    "Kashmir.Combat.HitEvidence.RejectNonBlocking",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHitEvidenceRejectNonBlockingTest::RunTest(
    const FString& Parameters)
{
    FHitResult Hit;
    Hit.bBlockingHit = false;

    FKashmirHitEvidenceBuildInput Input;

    Input.InstigatorId =
        TEXT("Player");

    Input.TargetId =
        TEXT("Enemy_01");

    FKashmirHitEvidence Evidence;
    FString Reason;

    const bool bBuilt =
        FKashmirHitEvidenceBuilder::BuildFromHitResult(
            Hit,
            Input,
            nullptr,
            Evidence,
            Reason
        );

    TestFalse(
        TEXT("Non-blocking hit is rejected"),
        bBuilt
    );

    TestEqual(
        TEXT("Rejected hit explains reason"),
        Reason,
        FString(
            TEXT("hit evidence requires a blocking hit")
        )
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHitEvidenceRejectUnknownBoneTest,
    "Kashmir.Combat.HitEvidence.RejectUnknownBone",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHitEvidenceRejectUnknownBoneTest::RunTest(
    const FString& Parameters)
{
    FHitResult Hit;
    Hit.bBlockingHit = true;

    Hit.ImpactPoint =
        FVector::ZeroVector;

    Hit.ImpactNormal =
        FVector::UpVector;

    Hit.BoneName =
        TEXT("mystery_bone");

    FKashmirHitEvidenceBuildInput Input;

    Input.InstigatorId =
        TEXT("Player");

    Input.TargetId =
        TEXT("Enemy_01");

    UKashmirHitRegionMap* RegionMap =
        NewObject<UKashmirHitRegionMap>();

    FKashmirHitEvidence Evidence;
    FString Reason;

    const bool bBuilt =
        FKashmirHitEvidenceBuilder::BuildFromHitResult(
            Hit,
            Input,
            RegionMap,
            Evidence,
            Reason
        );

    TestFalse(
        TEXT("Unmapped skeletal bone is rejected"),
        bBuilt
    );

    TestEqual(
        TEXT("Unmapped bone explains reason"),
        Reason,
        FString(
            TEXT("hit bone is not mapped to a combat region")
        )
    );

    return true;
}

#endif