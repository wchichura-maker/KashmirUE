#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDefenseResolver.h"
#include "Misc/AutomationTest.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDefenseResolverGeometryTest,
    "Kashmir.Combat.Defense.Block.GeometryMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)


namespace
{
    FKashmirHitEvidence MakeDefenseEvidence(
        const FVector& AttackDirection)
    {
        FKashmirHitEvidence Evidence;

        Evidence.InstigatorId =
            TEXT("Attacker");

        Evidence.TargetId =
            TEXT("Defender");

        Evidence.ContactSource =
            EKashmirContactSourceType::Weapon;

        Evidence.SourceId =
            TEXT("Sword");

        Evidence.ImpactPoint =
            FVector::ZeroVector;

        Evidence.ImpactNormal =
            FVector::UpVector;

        Evidence.ContactVelocity =
            AttackDirection.GetSafeNormal() *
            1000.0f;

        Evidence.AttackDirection =
            AttackDirection;

        Evidence.RelativeSpeed =
            1000.0f;

        return Evidence;
    }
}


bool FKashmirDefenseResolverGeometryTest::RunTest(
    const FString& Parameters)
{
    struct FBlockCase
    {
        const TCHAR* Name;
        FVector AttackDirection;
        bool bBlocking;
        bool bExpectedBlocked;
    };

    /*
     * Defender faces +X.
     *
     * HalfAngle = 60 degrees:
     * total defensive arc = 120 degrees.
     */
    const TArray<FBlockCase> Cases =
    {
        {
            TEXT("DirectFront"),
            FVector(-1.0f, 0.0f, 0.0f),
            true,
            true
        },

        {
            TEXT("FrontRight45"),
            FVector(-1.0f, -1.0f, 0.0f),
            true,
            true
        },

        {
            TEXT("FrontLeft45"),
            FVector(-1.0f, 1.0f, 0.0f),
            true,
            true
        },

        {
            TEXT("SideRight90"),
            FVector(0.0f, -1.0f, 0.0f),
            true,
            false
        },

        {
            TEXT("SideLeft90"),
            FVector(0.0f, 1.0f, 0.0f),
            true,
            false
        },

        {
            TEXT("DirectRear"),
            FVector(1.0f, 0.0f, 0.0f),
            true,
            false
        },

        {
            TEXT("FrontButGuardInactive"),
            FVector(-1.0f, 0.0f, 0.0f),
            false,
            false
        }
    };

    FKashmirDefenseResolver Resolver;

    int32 SuccessfulCases =
        0;

    for (const FBlockCase& TestCase :
        Cases)
    {
        const FKashmirHitEvidence Evidence =
            MakeDefenseEvidence(
                TestCase.AttackDirection
            );

        FKashmirBlockState BlockState;

        BlockState.bActive =
            TestCase.bBlocking;

        BlockState.ForwardDirection =
            FVector::ForwardVector;

        BlockState.HalfAngleDegrees =
            60.0f;

        FKashmirDefenseResult Result;
        FString Reason;

        const bool bResolved =
            Resolver.ResolveBlock(
                Evidence,
                BlockState,
                Result,
                Reason
            );

        const FString Prefix =
            FString::Printf(
                TEXT("[%s]"),
                TestCase.Name
            );

        TestTrue(
            *FString::Printf(
                TEXT("%s resolution succeeds"),
                *Prefix
            ),
            bResolved
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s produces no validation error"),
                *Prefix
            ),
            Reason.IsEmpty()
        );

        const bool bExpected =
            Result.bBlocked ==
            TestCase.bExpectedBlocked;

        TestTrue(
            *FString::Printf(
                TEXT("%s produces expected block result"),
                *Prefix
            ),
            bExpected
        );

        AddInfo(
            FString::Printf(
                TEXT(
                    "%s blocked=%s "
                    "alignment=%.4f "
                    "required=%.4f"
                ),
                *Prefix,
                Result.bBlocked
                    ? TEXT("true")
                    : TEXT("false"),
                Result.Alignment,
                Result.RequiredAlignment
            )
        );

        if (
            bResolved &&
            Reason.IsEmpty() &&
            bExpected)
        {
            ++SuccessfulCases;
        }
    }

    TestEqual(
        TEXT(
            "All geometric block cases resolve correctly"
        ),
        SuccessfulCases,
        Cases.Num()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDefenseResolverBoundaryTest,
    "Kashmir.Combat.Defense.Block.AngleBoundary",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirDefenseResolverBoundaryTest::RunTest(
    const FString& Parameters)
{
    FKashmirDefenseResolver Resolver;

    FKashmirBlockState BlockState;

    BlockState.bActive =
        true;

    BlockState.ForwardDirection =
        FVector::ForwardVector;

    BlockState.HalfAngleDegrees =
        60.0f;

    /*
     * Source exactly 60 degrees from forward.
     *
     * Attack travels from that source
     * toward the defender.
     */
    const FVector SourceDirection =
        FVector(
            0.5f,
            FMath::Sqrt(3.0f) * 0.5f,
            0.0f
        );

    const FKashmirHitEvidence Evidence =
        MakeDefenseEvidence(
            -SourceDirection
        );

    FKashmirDefenseResult Result;
    FString Reason;

    const bool bResolved =
        Resolver.ResolveBlock(
            Evidence,
            BlockState,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Boundary resolution succeeds"),
        bResolved
    );

    TestTrue(
        TEXT(
            "Attack exactly on block boundary is blocked"
        ),
        Result.bBlocked
    );

    TestTrue(
        TEXT(
            "Boundary alignment is approximately 0.5"
        ),
        FMath::IsNearlyEqual(
            Result.Alignment,
            0.5f,
            0.001f
        )
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDefenseResolverInvalidInputTest,
    "Kashmir.Combat.Defense.Block.RejectInvalidInput",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirDefenseResolverInvalidInputTest::RunTest(
    const FString& Parameters)
{
    FKashmirDefenseResolver Resolver;

    FKashmirHitEvidence Evidence =
        MakeDefenseEvidence(
            FVector::ZeroVector
        );

    FKashmirBlockState BlockState;

    BlockState.bActive =
        true;

    FKashmirDefenseResult Result;
    FString Reason;

    const bool bResolved =
        Resolver.ResolveBlock(
            Evidence,
            BlockState,
            Result,
            Reason
        );

    TestFalse(
        TEXT(
            "Zero attack direction is rejected"
        ),
        bResolved
    );

    TestTrue(
        TEXT(
            "Invalid attack direction produces reason"
        ),
        Reason.Contains(
            TEXT("attack direction")
        )
    );

    return true;
}

#endif