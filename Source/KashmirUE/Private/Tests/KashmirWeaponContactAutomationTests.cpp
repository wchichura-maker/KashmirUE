#include "Misc/AutomationTest.h"

#include "Combat/KashmirWeaponContact.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirWeaponContactBuildSegmentsTest,
    "Kashmir.Combat.WeaponContact.BuildSegments",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirWeaponContactBuildSegmentsTest::RunTest(
    const FString& Parameters)
{
    FKashmirWeaponContactFrame PreviousFrame;
    FKashmirWeaponContactFrame CurrentFrame;

    FKashmirWeaponContactPoint PreviousBase;
    PreviousBase.Id = TEXT("Base");
    PreviousBase.Position =
        FVector(0.0, 0.0, 0.0);

    FKashmirWeaponContactPoint PreviousTip;
    PreviousTip.Id = TEXT("Tip");
    PreviousTip.Position =
        FVector(0.0, 100.0, 0.0);

    PreviousFrame.Points.Add(PreviousBase);
    PreviousFrame.Points.Add(PreviousTip);

    FKashmirWeaponContactPoint CurrentBase;
    CurrentBase.Id = TEXT("Base");
    CurrentBase.Position =
        FVector(10.0, 0.0, 0.0);

    FKashmirWeaponContactPoint CurrentTip;
    CurrentTip.Id = TEXT("Tip");
    CurrentTip.Position =
        FVector(10.0, 100.0, 0.0);

    CurrentFrame.Points.Add(CurrentBase);
    CurrentFrame.Points.Add(CurrentTip);

    TArray<FKashmirWeaponSweepSegment> Segments;
    FString Reason;

    const bool bBuilt =
        FKashmirWeaponContactBuilder::BuildSweepSegments(
            PreviousFrame,
            CurrentFrame,
            Segments,
            Reason
        );

    TestTrue(
        TEXT("Matching frames build sweep segments"),
        bBuilt
    );

    TestEqual(
        TEXT("Two contact points produce two segments"),
        Segments.Num(),
        2
    );

    TestEqual(
        TEXT("Base segment id is preserved"),
        Segments[0].PointId,
        FName(TEXT("Base"))
    );

    TestTrue(
        TEXT("Base moved ten units"),
        FMath::IsNearlyEqual(
            Segments[0].Distance,
            10.0f
        )
    );

    TestEqual(
        TEXT("Tip segment id is preserved"),
        Segments[1].PointId,
        FName(TEXT("Tip"))
    );

    TestTrue(
        TEXT("Tip moved ten units"),
        FMath::IsNearlyEqual(
            Segments[1].Distance,
            10.0f
        )
    );

    TestTrue(
        TEXT("Build succeeds without error"),
        Reason.IsEmpty()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirWeaponContactRejectMismatchedFramesTest,
    "Kashmir.Combat.WeaponContact.RejectMismatchedFrames",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirWeaponContactRejectMismatchedFramesTest::RunTest(
    const FString& Parameters)
{
    FKashmirWeaponContactFrame PreviousFrame;
    FKashmirWeaponContactFrame CurrentFrame;

    FKashmirWeaponContactPoint Base;
    Base.Id = TEXT("Base");
    Base.Position = FVector::ZeroVector;

    FKashmirWeaponContactPoint Tip;
    Tip.Id = TEXT("Tip");
    Tip.Position =
        FVector(0.0, 100.0, 0.0);

    PreviousFrame.Points.Add(Base);
    PreviousFrame.Points.Add(Tip);

    CurrentFrame.Points.Add(Base);

    TArray<FKashmirWeaponSweepSegment> Segments;
    FString Reason;

    const bool bBuilt =
        FKashmirWeaponContactBuilder::BuildSweepSegments(
            PreviousFrame,
            CurrentFrame,
            Segments,
            Reason
        );

    TestFalse(
        TEXT("Mismatched contact point sets are rejected"),
        bBuilt
    );

    TestEqual(
        TEXT("Mismatched frames explain reason"),
        Reason,
        FString(
            TEXT("weapon contact frame point sets do not match")
        )
    );

    TestEqual(
        TEXT("Rejected build returns no segments"),
        Segments.Num(),
        0
    );

    return true;
}

#endif