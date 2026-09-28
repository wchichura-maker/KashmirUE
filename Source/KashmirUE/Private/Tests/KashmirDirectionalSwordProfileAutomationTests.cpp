#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Misc/AutomationTest.h"


namespace
{
    FKashmirSwordAuthoredAction MakeAuthoredAction(FName ActionId)
    {
        FKashmirSwordAuthoredAction Action;
        Action.ActionId = ActionId;
        Action.Montage = TSoftObjectPtr<UAnimMontage>(
            FSoftObjectPath(TEXT("/Game/KashmirAct/Test/AM_Sword_Test.AM_Sword_Test")));
        Action.MontageSection = TEXT("Attack");
        Action.PlayRate = 1.25f;
        Action.StartupDuration = 0.20f;
        Action.ActiveDuration = 0.15f;
        Action.RecoveryDuration = 0.30f;
        Action.PoseConfig.ActiveFootLockAlpha = 0.60f;
        Action.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Action.BaseDamage = 25.0f;
        Action.AttackPowerMultiplier = 1.1f;
        return Action;
    }

    UKashmirDirectionalSwordProfile* MakeValidProfile()
    {
        UKashmirDirectionalSwordProfile* Profile =
            NewObject<UKashmirDirectionalSwordProfile>();
        Profile->GestureConfig.MinimumDragDistance = 10.0f;
        Profile->GestureConfig.FullIntensityDistance = 100.0f;
        Profile->GestureConfig.CurvedFamilyThreshold = 0.10f;

        FKashmirSwordActionBinding Binding;
        Binding.Family = EKashmirSwordGestureFamily::Direct;
        Binding.Direction = EKashmirSwordGestureDirection::Right;
        Binding.ActionId = TEXT("Sword.Direct.Right");
        Profile->GestureConfig.ActionBindings.Add(Binding);
        Profile->Actions.Add(MakeAuthoredAction(Binding.ActionId));
        return Profile;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordProfileResolveTest,
    "Kashmir.Combat.DirectionalSword.Profile.ResolveActionPlan",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordProfileResolveTest::RunTest(const FString& Parameters)
{
    const UKashmirDirectionalSwordProfile* Profile = MakeValidProfile();
    FString Reason;
    TestTrue(TEXT("Directional sword profile validates"), Profile->ValidateProfile(Reason));

    FKashmirSwordGestureInput Input;
    Input.Samples = { FVector2D::ZeroVector, FVector2D(50.0f, 0.0f) };
    Input.DurationSeconds = 0.25f;
    FKashmirSwordActionPlan Plan;
    TestTrue(TEXT("Action plan resolves"), Profile->ResolveActionPlan(Input, Plan, Reason));
    TestTrue(TEXT("Resolved plan is explicit"), Plan.bResolved);
    TestEqual(TEXT("Plan preserves melee archetype"),
        Plan.ArchetypeDefinition.Archetype,
        EKashmirMeleeArchetype::OneHandedSword);
    TestEqual(TEXT("Plan keeps gesture action id"), Plan.RuntimeDefinition.ActionId, FName(TEXT("Sword.Direct.Right")));
    TestEqual(TEXT("Startup comes from authored action"), Plan.RuntimeDefinition.StartupDuration, 0.20f);
    TestEqual(TEXT("Active window comes from authored action"), Plan.RuntimeDefinition.ActiveDuration, 0.15f);
    TestEqual(TEXT("Recovery comes from authored action"), Plan.RuntimeDefinition.RecoveryDuration, 0.30f);
    TestEqual(TEXT("Montage section is preserved"), Plan.MontageSection, FName(TEXT("Attack")));
    TestEqual(TEXT("Play rate is preserved"), Plan.PlayRate, 1.25f);
    TestEqual(TEXT("Pose constraints are preserved"), Plan.PoseConfig.ActiveFootLockAlpha, 0.60f);
    TestEqual(TEXT("Base damage is preserved"), Plan.BaseDamage, 25.0f);
    TestEqual(TEXT("Attack multiplier is preserved"), Plan.AttackPowerMultiplier, 1.1f);
    TestTrue(TEXT("Combat effect is preserved"),
        Plan.CombatDefinition.Effects.Contains(EKashmirResolutionType::Damage));
    TestEqual(TEXT("Gesture intensity reaches request"), Plan.Gesture.ActionRequest.Intensity, 0.5f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalSwordProfileInvalidTest,
    "Kashmir.Combat.DirectionalSword.Profile.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalSwordProfileInvalidTest::RunTest(const FString& Parameters)
{
    FString Reason;

    UKashmirDirectionalSwordProfile* Profile = MakeValidProfile();
    Profile->Actions[0].Montage.Reset();
    TestFalse(TEXT("Missing montage is rejected"), Profile->ValidateProfile(Reason));

    Profile = MakeValidProfile();
    Profile->Actions[0].PlayRate = 0.0f;
    TestFalse(TEXT("Zero play rate is rejected"), Profile->ValidateProfile(Reason));

    Profile = MakeValidProfile();
    Profile->Actions[0].ActiveDuration = -1.0f;
    TestFalse(TEXT("Invalid timeline is rejected"), Profile->ValidateProfile(Reason));

    Profile = MakeValidProfile();
    Profile->Actions[0].PoseConfig.ActiveFootLockAlpha = 2.0f;
    TestFalse(TEXT("Invalid pose constraints are rejected"), Profile->ValidateProfile(Reason));

    Profile = MakeValidProfile();
    const FKashmirSwordAuthoredAction Duplicate = Profile->Actions[0];
    Profile->Actions.Add(Duplicate);
    TestFalse(TEXT("Duplicate action id is rejected"), Profile->ValidateProfile(Reason));

    Profile = MakeValidProfile();
    Profile->GestureConfig.ActionBindings[0].ActionId = TEXT("Sword.Unknown");
    TestFalse(TEXT("Unknown binding action is rejected"), Profile->ValidateProfile(Reason));

    Profile = MakeValidProfile();
    Profile->ArchetypeDefinition.ContactPointIds.Empty();
    TestFalse(TEXT("Invalid melee archetype is rejected"),
        Profile->ValidateProfile(Reason));
    return true;
}

#endif
