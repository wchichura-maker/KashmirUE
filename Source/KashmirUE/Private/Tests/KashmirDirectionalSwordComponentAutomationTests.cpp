#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Misc/AutomationTest.h"

#include <limits>


namespace
{
    UKashmirDirectionalSwordProfile* MakeRuntimeProfile()
    {
        UKashmirDirectionalSwordProfile* Profile =
            NewObject<UKashmirDirectionalSwordProfile>();
        Profile->GestureConfig.MinimumDragDistance = 10.0f;
        Profile->GestureConfig.FullIntensityDistance = 100.0f;

        FKashmirSwordActionBinding Binding;
        Binding.Family = EKashmirSwordGestureFamily::Direct;
        Binding.Direction = EKashmirSwordGestureDirection::Right;
        Binding.ActionId = TEXT("Sword.Direct.Right");
        Profile->GestureConfig.ActionBindings.Add(Binding);

        FKashmirSwordAuthoredAction Action;
        Action.ActionId = Binding.ActionId;
        Action.Montage = TSoftObjectPtr<UAnimMontage>(
            FSoftObjectPath(TEXT("/Game/KashmirAct/Test/AM_Sword_Test.AM_Sword_Test")));
        Action.StartupDuration = 0.10f;
        Action.ActiveDuration = 0.10f;
        Action.RecoveryDuration = 0.10f;
        Action.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Action.BaseDamage = 20.0f;
        Profile->Actions.Add(Action);
        return Profile;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordComponentCaptureTest,
    "Kashmir.Combat.DirectionalSword.Runtime.CaptureAndAdvance",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordComponentCaptureTest::RunTest(
    const FString& Parameters)
{
    UKashmirDirectionalSwordComponent* Component =
        NewObject<UKashmirDirectionalSwordComponent>();
    Component->SetProfile(MakeRuntimeProfile());

    FString Reason;
    TestTrue(TEXT("Gesture capture begins"), Component->BeginGesture(Reason));
    TestTrue(TEXT("First drag sample is accepted"),
        Component->AddGestureDelta(FVector2D(20.0f, 0.0f), 0.05f, Reason));
    TestTrue(TEXT("Second drag sample is accepted"),
        Component->AddGestureDelta(FVector2D(30.0f, 0.0f), 0.05f, Reason));
    TestTrue(TEXT("Gesture starts authoritative action"),
        Component->CompleteGesture(Reason));
    TestFalse(TEXT("Capture ends after resolution"), Component->IsCapturingGesture());
    TestTrue(TEXT("Resolved action is active"), Component->GetRuntimeState().bActive);
    TestEqual(TEXT("Resolved action id is retained"),
        Component->GetRuntimeState().ActionId,
        FName(TEXT("Sword.Direct.Right")));
    TestTrue(TEXT("Resolved plan remains available"), Component->GetActivePlan().bResolved);

    TestTrue(TEXT("Runtime advances"), Component->AdvanceRuntime(0.31f, Reason));
    TestFalse(TEXT("Runtime completes deterministically"),
        Component->GetRuntimeState().bActive);
    TestFalse(TEXT("Completed plan is released"), Component->GetActivePlan().bResolved);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordComponentRejectInvalidTest,
    "Kashmir.Combat.DirectionalSword.Runtime.RejectInvalidGesture",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordComponentRejectInvalidTest::RunTest(
    const FString& Parameters)
{
    UKashmirDirectionalSwordComponent* Component =
        NewObject<UKashmirDirectionalSwordComponent>();
    Component->SetProfile(MakeRuntimeProfile());
    FString Reason;

    TestFalse(TEXT("Delta without capture is rejected"),
        Component->AddGestureDelta(FVector2D(20.0f, 0.0f), 0.05f, Reason));

    TestTrue(TEXT("Capture begins"), Component->BeginGesture(Reason));
    TestFalse(TEXT("Non-finite delta is rejected"),
        Component->AddGestureDelta(
            FVector2D(std::numeric_limits<double>::quiet_NaN(), 0.0),
            0.05f,
            Reason));
    Component->CancelGesture();

    TestTrue(TEXT("Short capture begins"), Component->BeginGesture(Reason));
    TestTrue(TEXT("Short delta is recorded"),
        Component->AddGestureDelta(FVector2D(2.0f, 0.0f), 0.05f, Reason));
    TestFalse(TEXT("Below-threshold gesture is rejected"),
        Component->CompleteGesture(Reason));
    TestFalse(TEXT("Rejected gesture starts no action"),
        Component->GetRuntimeState().bActive);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordSharedRequestTest,
    "Kashmir.Combat.DirectionalSword.Runtime.SharedActionRequest",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordSharedRequestTest::RunTest(
    const FString& Parameters)
{
    UKashmirDirectionalSwordComponent* Component =
        NewObject<UKashmirDirectionalSwordComponent>();
    Component->SetProfile(MakeRuntimeProfile());

    FKashmirActionRequest Request;
    Request.ActionId = TEXT("Sword.Direct.Right");
    Request.Direction = FVector2D::UnitX();
    Request.Intensity = 0.75f;
    Request.Curvature = 0.0f;

    FString Reason;
    TestTrue(TEXT("External shared request starts the same runtime"),
        Component->StartActionRequest(Request, Reason));
    TestEqual(TEXT("External request keeps action identity"),
        Component->GetRuntimeState().ActionId, Request.ActionId);
    TestEqual(TEXT("External request reaches the active plan"),
        Component->GetActivePlan().Gesture.ActionRequest.Intensity,
        Request.Intensity);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordTraceWindowTest,
    "Kashmir.Combat.DirectionalSword.Runtime.AuthoritativeTraceWindow",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordTraceWindowTest::RunTest(
    const FString& Parameters)
{
    UKashmirDirectionalSwordComponent* Component =
        NewObject<UKashmirDirectionalSwordComponent>();
    UKashmirWeaponTraceComponent* Trace =
        NewObject<UKashmirWeaponTraceComponent>();
    Component->SetProfile(MakeRuntimeProfile());
    Component->SetWeaponTraceComponent(Trace);

    FString Reason;
    TestTrue(TEXT("Gesture capture begins"), Component->BeginGesture(Reason));
    TestTrue(TEXT("Gesture sample is accepted"),
        Component->AddGestureDelta(FVector2D(50.0f, 0.0f), 0.10f, Reason));
    TestTrue(TEXT("Gesture starts action"), Component->CompleteGesture(Reason));
    TestFalse(TEXT("Startup keeps trace closed"), Trace->IsTraceWindowActive());

    TestTrue(TEXT("Runtime reaches active phase"),
        Component->AdvanceRuntime(0.10f, Reason));
    TestEqual(TEXT("Runtime is active"),
        Component->GetRuntimeState().Phase, EKashmirActionPhase::Active);
    TestTrue(TEXT("Active phase opens trace"), Trace->IsTraceWindowActive());

    TestTrue(TEXT("Runtime reaches recovery"),
        Component->AdvanceRuntime(0.10f, Reason));
    TestEqual(TEXT("Runtime is recovering"),
        Component->GetRuntimeState().Phase, EKashmirActionPhase::Recovery);
    TestFalse(TEXT("Recovery closes trace"), Trace->IsTraceWindowActive());

    TestTrue(TEXT("Runtime completes"),
        Component->AdvanceRuntime(0.10f, Reason));
    TestFalse(TEXT("Completed action keeps trace closed"),
        Trace->IsTraceWindowActive());
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordContactResolverTest,
    "Kashmir.Combat.DirectionalSword.Contact.Resolution",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordContactResolverTest::RunTest(
    const FString& Parameters)
{
    const UKashmirDirectionalSwordProfile* Profile = MakeRuntimeProfile();
    FKashmirSwordGestureInput Gesture;
    Gesture.Samples = { FVector2D::ZeroVector, FVector2D(50.0f, 0.0f) };
    Gesture.DurationSeconds = 0.10f;

    FString Reason;
    FKashmirSwordActionPlan Plan;
    TestTrue(TEXT("Action plan resolves"),
        Profile->ResolveActionPlan(Gesture, Plan, Reason));

    FKashmirWeaponTraceHit TraceHit;
    TraceHit.ContactPointId = TEXT("Weapon_Tip");
    TraceHit.ContactVelocity = FVector(500.0f, 0.0f, 0.0f);
    TraceHit.AttackDirection = FVector::ForwardVector;
    TraceHit.Speed = 500.0f;

    FKashmirDirectionalSwordContactResolver Resolver;
    FKashmirDirectionalSwordContact Contact;
    TestTrue(TEXT("Physical trace and authored action resolve contact"),
        Resolver.Resolve(TraceHit, Plan, Contact, Reason));
    TestTrue(TEXT("Contact is explicit"), Contact.bResolved);
    TestEqual(TEXT("Contact keeps action id"), Contact.ActionId,
        FName(TEXT("Sword.Direct.Right")));
    TestEqual(TEXT("Contact carries stable style id"), Contact.StyleId,
        FName(TEXT("Melee.OneHandedSword")));
    TestEqual(TEXT("Contact carries stable source id"), Contact.SourceId,
        FName(TEXT("Weapon.MainHand")));
    TestEqual(TEXT("Contact carries weapon evidence semantics"),
        Contact.ContactSource, EKashmirContactSourceType::Weapon);
    TestEqual(TEXT("Contact keeps base damage"), Contact.BaseDamage, 20.0f);
    TestTrue(TEXT("Contact keeps logical damage effect"),
        Contact.CombatDefinition.Effects.Contains(EKashmirResolutionType::Damage));

    TraceHit.Speed = -1.0f;
    TestFalse(TEXT("Invalid physical evidence is rejected"),
        Resolver.Resolve(TraceHit, Plan, Contact, Reason));
    return true;
}

#endif
