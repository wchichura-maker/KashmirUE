#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDirectionalSwordResolver.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    FKashmirDirectionalSwordConfig MakeSwordConfig()
    {
        FKashmirDirectionalSwordConfig Config;
        Config.MinimumDragDistance = 10.0f;
        Config.FullIntensityDistance = 100.0f;
        Config.CurvedFamilyThreshold = 0.10f;

        FKashmirSwordActionBinding DirectRight;
        DirectRight.Family = EKashmirSwordGestureFamily::Direct;
        DirectRight.Direction = EKashmirSwordGestureDirection::Right;
        DirectRight.ActionId = TEXT("Sword.Direct.Right");
        Config.ActionBindings.Add(DirectRight);

        FKashmirSwordActionBinding CurvedRight = DirectRight;
        CurvedRight.Family = EKashmirSwordGestureFamily::Curved;
        CurvedRight.ActionId = TEXT("Sword.Curved.Right");
        Config.ActionBindings.Add(CurvedRight);

        return Config;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordResolverMatrixTest,
    "Kashmir.Combat.DirectionalSword.ResolutionMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordResolverMatrixTest::RunTest(const FString& Parameters)
{
    FKashmirDirectionalSwordResolver Resolver;
    const FKashmirDirectionalSwordConfig Config = MakeSwordConfig();

    FKashmirSwordGestureInput DirectInput;
    DirectInput.Samples = { FVector2D(0.0f, 0.0f), FVector2D(50.0f, 0.0f) };
    DirectInput.DurationSeconds = 0.25f;
    FKashmirDirectionalSwordResult Result;
    FString Reason;
    TestTrue(TEXT("Direct gesture resolves"), Resolver.Resolve(DirectInput, Config, Result, Reason));
    TestTrue(TEXT("Direct gesture is explicit"), Result.bGestureResolved);
    TestEqual(TEXT("Direct family resolves"), Result.Family, EKashmirSwordGestureFamily::Direct);
    TestEqual(TEXT("Right direction resolves"), Result.Direction, EKashmirSwordGestureDirection::Right);
    TestEqual(TEXT("Half distance gives half intensity"), Result.Intensity, 0.5f);
    TestEqual(TEXT("Straight gesture has zero curvature"), Result.Curvature, 0.0f);
    TestEqual(TEXT("Direct binding supplies action"), Result.ActionRequest.ActionId, FName(TEXT("Sword.Direct.Right")));
    TestTrue(TEXT("Action request remains valid"), Result.ActionRequest.IsValid(Reason));

    FKashmirSwordGestureInput CurvedInput;
    CurvedInput.Samples =
    {
        FVector2D(0.0f, 0.0f),
        FVector2D(25.0f, 25.0f),
        FVector2D(50.0f, 0.0f)
    };
    CurvedInput.DurationSeconds = 0.50f;
    TestTrue(TEXT("Curved gesture resolves"), Resolver.Resolve(CurvedInput, Config, Result, Reason));
    TestEqual(TEXT("Curved family resolves"), Result.Family, EKashmirSwordGestureFamily::Curved);
    TestTrue(TEXT("Curvature exceeds threshold"), Result.Curvature >= Config.CurvedFamilyThreshold);
    TestEqual(TEXT("Curved binding supplies action"), Result.ActionRequest.ActionId, FName(TEXT("Sword.Curved.Right")));

    FKashmirDirectionalSwordConfig FullConfig = Config;
    FullConfig.FullIntensityDistance = 25.0f;
    TestTrue(TEXT("Intensity clamp resolves"), Resolver.Resolve(DirectInput, FullConfig, Result, Reason));
    TestEqual(TEXT("Intensity clamps to one"), Result.Intensity, 1.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordDirectionsTest,
    "Kashmir.Combat.DirectionalSword.EightDirections",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordDirectionsTest::RunTest(const FString& Parameters)
{
    struct FCase
    {
        FVector2D End;
        EKashmirSwordGestureDirection Expected;
    };

    const TArray<FCase> Cases =
    {
        { FVector2D(100.0f, 0.0f), EKashmirSwordGestureDirection::Right },
        { FVector2D(100.0f, 100.0f), EKashmirSwordGestureDirection::UpRight },
        { FVector2D(0.0f, 100.0f), EKashmirSwordGestureDirection::Up },
        { FVector2D(-100.0f, 100.0f), EKashmirSwordGestureDirection::UpLeft },
        { FVector2D(-100.0f, 0.0f), EKashmirSwordGestureDirection::Left },
        { FVector2D(-100.0f, -100.0f), EKashmirSwordGestureDirection::DownLeft },
        { FVector2D(0.0f, -100.0f), EKashmirSwordGestureDirection::Down },
        { FVector2D(100.0f, -100.0f), EKashmirSwordGestureDirection::DownRight }
    };

    FKashmirDirectionalSwordResolver Resolver;
    for (const FCase& TestCase : Cases)
    {
        FKashmirDirectionalSwordConfig Config;
        Config.MinimumDragDistance = 1.0f;
        Config.FullIntensityDistance = 100.0f;
        Config.CurvedFamilyThreshold = 0.10f;
        FKashmirSwordActionBinding Binding;
        Binding.Family = EKashmirSwordGestureFamily::Direct;
        Binding.Direction = TestCase.Expected;
        Binding.ActionId = TEXT("Sword.Test");
        Config.ActionBindings.Add(Binding);

        FKashmirSwordGestureInput Input;
        Input.Samples = { FVector2D::ZeroVector, TestCase.End };
        Input.DurationSeconds = 0.25f;
        FKashmirDirectionalSwordResult Result;
        FString Reason;
        TestTrue(TEXT("Directional gesture resolves"), Resolver.Resolve(Input, Config, Result, Reason));
        TestEqual(TEXT("Direction sector is correct"), Result.Direction, TestCase.Expected);
    }

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordInvalidInputTest,
    "Kashmir.Combat.DirectionalSword.RejectInvalidInput",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordInvalidInputTest::RunTest(const FString& Parameters)
{
    FKashmirDirectionalSwordResolver Resolver;
    FKashmirDirectionalSwordConfig Config = MakeSwordConfig();
    FKashmirSwordGestureInput Input;
    Input.Samples = { FVector2D::ZeroVector, FVector2D(50.0f, 0.0f) };
    Input.DurationSeconds = 0.25f;
    FKashmirDirectionalSwordResult Result;
    FString Reason;

    Input.Samples.SetNum(1);
    TestFalse(TEXT("Single sample is rejected"), Resolver.Resolve(Input, Config, Result, Reason));

    Input.Samples = { FVector2D::ZeroVector, FVector2D(50.0f, 0.0f) };
    Input.DurationSeconds = 0.0f;
    TestFalse(TEXT("Zero duration is rejected"), Resolver.Resolve(Input, Config, Result, Reason));

    Input.DurationSeconds = 0.25f;
    Input.Samples[1].X = std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("Non-finite sample is rejected"), Resolver.Resolve(Input, Config, Result, Reason));

    Input.Samples = { FVector2D::ZeroVector, FVector2D(5.0f, 0.0f) };
    TestFalse(TEXT("Short drag is rejected"), Resolver.Resolve(Input, Config, Result, Reason));

    Input.Samples = { FVector2D::ZeroVector, FVector2D(50.0f, 0.0f) };
    Config.ActionBindings.Empty();
    TestFalse(TEXT("Missing binding is rejected"), Resolver.Resolve(Input, Config, Result, Reason));

    Config = MakeSwordConfig();
    const FKashmirSwordActionBinding DuplicateBinding = Config.ActionBindings[0];
    Config.ActionBindings.Add(DuplicateBinding);
    TestFalse(TEXT("Duplicate binding is rejected"), Resolver.Resolve(Input, Config, Result, Reason));

    Config = MakeSwordConfig();
    Config.FullIntensityDistance = Config.MinimumDragDistance - 1.0f;
    TestFalse(TEXT("Invalid distance range is rejected"), Resolver.Resolve(Input, Config, Result, Reason));
    return true;
}

#endif
