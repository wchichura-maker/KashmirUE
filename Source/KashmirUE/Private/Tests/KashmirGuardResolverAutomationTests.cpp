#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirGuardResolver.h"
#include "Misc/AutomationTest.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirGuardResolverMatrixTest,
    "Kashmir.Combat.Guard.ResolutionMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)


bool FKashmirGuardResolverMatrixTest::RunTest(
    const FString& Parameters)
{
    struct FGuardCase
    {
        const TCHAR* Name;

        bool bBlocked;

        float BaseGuardDamage;
        float Multiplier;
        float AvailableStamina;

        float ExpectedFinalGuardDamage;
        float ExpectedStaminaAfter;

        bool bExpectedProcessed;
        bool bExpectedBroken;
    };

    const TArray<FGuardCase> Cases =
    {
        {
            TEXT("EnoughStamina"),
            true,
            30.0f,
            1.0f,
            100.0f,
            30.0f,
            70.0f,
            true,
            false
        },

        {
            TEXT("ExactStamina"),
            true,
            30.0f,
            1.0f,
            30.0f,
            30.0f,
            0.0f,
            true,
            false
        },

        {
            TEXT("InsufficientStamina"),
            true,
            30.0f,
            1.0f,
            20.0f,
            30.0f,
            0.0f,
            true,
            true
        },

        {
            TEXT("GuardMultiplierHalf"),
            true,
            40.0f,
            0.5f,
            50.0f,
            20.0f,
            30.0f,
            true,
            false
        },

        {
            TEXT("GuardMultiplierDouble"),
            true,
            20.0f,
            2.0f,
            30.0f,
            40.0f,
            0.0f,
            true,
            true
        },

        {
            TEXT("ZeroGuardDamage"),
            true,
            0.0f,
            1.0f,
            25.0f,
            0.0f,
            25.0f,
            true,
            false
        },

        {
            TEXT("NotBlocked"),
            false,
            100.0f,
            1.0f,
            50.0f,
            0.0f,
            50.0f,
            false,
            false
        }
    };

    FKashmirGuardResolver Resolver;

    int32 SuccessfulCases =
        0;

    for (const FGuardCase& TestCase :
        Cases)
    {
        FKashmirGuardInput Input;

        Input.bBlocked =
            TestCase.bBlocked;

        Input.BaseGuardDamage =
            TestCase.BaseGuardDamage;

        Input.GuardDamageMultiplier =
            TestCase.Multiplier;

        Input.AvailableStamina =
            TestCase.AvailableStamina;

        FKashmirGuardResult Result;
        FString Reason;

        const bool bResolved =
            Resolver.Resolve(
                Input,
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
                TEXT("%s produces no error"),
                *Prefix
            ),
            Reason.IsEmpty()
        );

        const bool bCorrectFinalDamage =
            FMath::IsNearlyEqual(
                Result.FinalGuardDamage,
                TestCase.ExpectedFinalGuardDamage,
                0.001f
            );

        const bool bCorrectStamina =
            FMath::IsNearlyEqual(
                Result.StaminaAfter,
                TestCase.ExpectedStaminaAfter,
                0.001f
            );

        const bool bCorrectProcessed =
            Result.bGuardProcessed ==
            TestCase.bExpectedProcessed;

        const bool bCorrectBroken =
            Result.bGuardBroken ==
            TestCase.bExpectedBroken;

        TestTrue(
            *FString::Printf(
                TEXT(
                    "%s final guard damage is correct"
                ),
                *Prefix
            ),
            bCorrectFinalDamage
        );

        TestTrue(
            *FString::Printf(
                TEXT(
                    "%s stamina result is correct"
                ),
                *Prefix
            ),
            bCorrectStamina
        );

        TestTrue(
            *FString::Printf(
                TEXT(
                    "%s processed state is correct"
                ),
                *Prefix
            ),
            bCorrectProcessed
        );

        TestTrue(
            *FString::Printf(
                TEXT(
                    "%s guard break result is correct"
                ),
                *Prefix
            ),
            bCorrectBroken
        );

        AddInfo(
            FString::Printf(
                TEXT(
                    "%s blocked=%s "
                    "guardDamage=%.2f "
                    "staminaBefore=%.2f "
                    "staminaAfter=%.2f "
                    "guardBroken=%s"
                ),
                *Prefix,
                TestCase.bBlocked
                    ? TEXT("true")
                    : TEXT("false"),
                Result.FinalGuardDamage,
                Result.StaminaBefore,
                Result.StaminaAfter,
                Result.bGuardBroken
                    ? TEXT("true")
                    : TEXT("false")
            )
        );

        if (
            bResolved &&
            Reason.IsEmpty() &&
            bCorrectFinalDamage &&
            bCorrectStamina &&
            bCorrectProcessed &&
            bCorrectBroken)
        {
            ++SuccessfulCases;
        }
    }

    TestEqual(
        TEXT(
            "All guard resolution cases pass"
        ),
        SuccessfulCases,
        Cases.Num()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirGuardResolverAppliedDamageTest,
    "Kashmir.Combat.Guard.AppliedStaminaDamage",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirGuardResolverAppliedDamageTest::RunTest(
    const FString& Parameters)
{
    FKashmirGuardInput Input;

    Input.bBlocked =
        true;

    Input.BaseGuardDamage =
        50.0f;

    Input.GuardDamageMultiplier =
        1.0f;

    Input.AvailableStamina =
        20.0f;

    FKashmirGuardResult Result;
    FString Reason;

    FKashmirGuardResolver Resolver;

    const bool bResolved =
        Resolver.Resolve(
            Input,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Guard resolution succeeds"),
        bResolved
    );

    TestTrue(
        TEXT(
            "Only available stamina can actually be consumed"
        ),
        FMath::IsNearlyEqual(
            Result.AppliedStaminaDamage,
            20.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT(
            "Requested guard damage remains fifty"
        ),
        FMath::IsNearlyEqual(
            Result.FinalGuardDamage,
            50.0f,
            0.001f
        )
    );

    TestTrue(
        TEXT(
            "Insufficient stamina causes guard break"
        ),
        Result.bGuardBroken
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirGuardResolverInvalidInputTest,
    "Kashmir.Combat.Guard.RejectInvalidInput",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirGuardResolverInvalidInputTest::RunTest(
    const FString& Parameters)
{
    struct FInvalidCase
    {
        const TCHAR* Name;
        float BaseGuardDamage;
        float Multiplier;
        float Stamina;
    };

    const TArray<FInvalidCase> Cases =
    {
        {
            TEXT("NegativeGuardDamage"),
            -1.0f,
            1.0f,
            10.0f
        },

        {
            TEXT("NegativeMultiplier"),
            10.0f,
            -1.0f,
            10.0f
        },

        {
            TEXT("NegativeStamina"),
            10.0f,
            1.0f,
            -1.0f
        }
    };

    FKashmirGuardResolver Resolver;

    int32 RejectedCases =
        0;

    for (const FInvalidCase& TestCase :
        Cases)
    {
        FKashmirGuardInput Input;

        Input.bBlocked =
            true;

        Input.BaseGuardDamage =
            TestCase.BaseGuardDamage;

        Input.GuardDamageMultiplier =
            TestCase.Multiplier;

        Input.AvailableStamina =
            TestCase.Stamina;

        FKashmirGuardResult Result;
        FString Reason;

        const bool bResolved =
            Resolver.Resolve(
                Input,
                Result,
                Reason
            );

        TestFalse(
            *FString::Printf(
                TEXT(
                    "[%s] invalid guard input is rejected"
                ),
                TestCase.Name
            ),
            bResolved
        );

        TestFalse(
            *FString::Printf(
                TEXT(
                    "[%s] rejection provides reason"
                ),
                TestCase.Name
            ),
            Reason.IsEmpty()
        );

        if (
            !bResolved &&
            !Reason.IsEmpty())
        {
            ++RejectedCases;
        }
    }

    TestEqual(
        TEXT(
            "All invalid guard inputs are rejected"
        ),
        RejectedCases,
        Cases.Num()
    );

    return true;
}

#endif