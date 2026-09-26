#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirClashResolver.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirHitEvidence MakeClashEvidence(
        const FName InstigatorId,
        const FVector& ContactVelocity,
        const EKashmirContactSourceType Source = EKashmirContactSourceType::Weapon)
    {
        FKashmirHitEvidence Evidence;
        Evidence.InstigatorId = InstigatorId;
        Evidence.TargetId = TEXT("WeaponContact");
        Evidence.ContactSource = Source;
        Evidence.SourceId = TEXT("Weapon");
        Evidence.ImpactNormal = FVector::UpVector;
        Evidence.ContactVelocity = ContactVelocity;
        Evidence.AttackDirection = ContactVelocity.GetSafeNormal();
        Evidence.RelativeSpeed = ContactVelocity.Size();
        return Evidence;
    }

    FKashmirClashInput MakeClashInput()
    {
        FKashmirClashInput Input;
        Input.FirstEvidence = MakeClashEvidence(TEXT("First"), FVector(100.0f, 0.0f, 0.0f));
        Input.SecondEvidence = MakeClashEvidence(TEXT("Second"), FVector(-100.0f, 0.0f, 0.0f));
        Input.bFirstAttackActive = true;
        Input.bSecondAttackActive = true;
        Input.MinimumRelativeSpeed = 200.0f;
        return Input;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirClashResolverMatrixTest,
    "Kashmir.Combat.Clash.ResolutionMatrix",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirClashResolverMatrixTest::RunTest(const FString& Parameters)
{
    struct FCase
    {
        const TCHAR* Name;
        TFunction<void(FKashmirClashInput&)> Mutate;
        bool bExpectedClash;
    };

    const TArray<FCase> Cases =
    {
        { TEXT("OpposingActiveWeapons"), [](FKashmirClashInput&) {}, true },
        { TEXT("ExactThreshold"), [](FKashmirClashInput& Input) { Input.MinimumRelativeSpeed = 200.0f; }, true },
        { TEXT("FirstInactive"), [](FKashmirClashInput& Input) { Input.bFirstAttackActive = false; }, false },
        { TEXT("SecondInactive"), [](FKashmirClashInput& Input) { Input.bSecondAttackActive = false; }, false },
        { TEXT("FirstNotWeapon"), [](FKashmirClashInput& Input) { Input.FirstEvidence.ContactSource = EKashmirContactSourceType::Hand; }, false },
        { TEXT("SecondNotWeapon"), [](FKashmirClashInput& Input) { Input.SecondEvidence.ContactSource = EKashmirContactSourceType::Environment; }, false },
        { TEXT("SameInstigator"), [](FKashmirClashInput& Input) { Input.SecondEvidence.InstigatorId = Input.FirstEvidence.InstigatorId; }, false },
        { TEXT("BelowThreshold"), [](FKashmirClashInput& Input) { Input.MinimumRelativeSpeed = 200.01f; }, false }
    };

    FKashmirClashResolver Resolver;
    for (const FCase& TestCase : Cases)
    {
        FKashmirClashInput Input = MakeClashInput();
        TestCase.Mutate(Input);
        FKashmirClashResult Result;
        FString Reason;
        const bool bResolved = Resolver.Resolve(Input, Result, Reason);
        TestTrue(*FString::Printf(TEXT("[%s] resolves"), TestCase.Name), bResolved);
        TestTrue(*FString::Printf(TEXT("[%s] has no error"), TestCase.Name), Reason.IsEmpty());
        TestEqual(*FString::Printf(TEXT("[%s] clash result"), TestCase.Name), Result.bClashed, TestCase.bExpectedClash);
        TestEqual(*FString::Printf(TEXT("[%s] relative speed"), TestCase.Name), Result.RelativeSpeed, 200.0f);
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirClashResolverInvalidInputTest,
    "Kashmir.Combat.Clash.RejectInvalidInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirClashResolverInvalidInputTest::RunTest(const FString& Parameters)
{
    struct FCase
    {
        const TCHAR* Name;
        TFunction<void(FKashmirClashInput&)> Mutate;
    };

    const TArray<FCase> Cases =
    {
        { TEXT("NegativeThreshold"), [](FKashmirClashInput& Input) { Input.MinimumRelativeSpeed = -1.0f; } },
        { TEXT("NaNThreshold"), [](FKashmirClashInput& Input) { Input.MinimumRelativeSpeed = std::numeric_limits<float>::quiet_NaN(); } },
        { TEXT("InfiniteThreshold"), [](FKashmirClashInput& Input) { Input.MinimumRelativeSpeed = std::numeric_limits<float>::infinity(); } },
        { TEXT("InvalidFirstEvidence"), [](FKashmirClashInput& Input) { Input.FirstEvidence.InstigatorId = NAME_None; } },
        { TEXT("InvalidSecondEvidence"), [](FKashmirClashInput& Input) { Input.SecondEvidence.TargetId = NAME_None; } }
    };

    FKashmirClashResolver Resolver;
    for (const FCase& TestCase : Cases)
    {
        FKashmirClashInput Input = MakeClashInput();
        TestCase.Mutate(Input);
        FKashmirClashResult Result;
        FString Reason;
        TestFalse(*FString::Printf(TEXT("[%s] rejected"), TestCase.Name), Resolver.Resolve(Input, Result, Reason));
        TestFalse(*FString::Printf(TEXT("[%s] explains rejection"), TestCase.Name), Reason.IsEmpty());
        TestFalse(*FString::Printf(TEXT("[%s] cannot clash"), TestCase.Name), Result.bClashed);
    }
    return true;
}

#endif
