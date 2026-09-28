#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirDirectionalMeleeResolver.h"
#include "Combat/KashmirCombatantComponent.h"
#include "Combat/KashmirHitRegionMap.h"
#include "Misc/AutomationTest.h"


namespace
{
    FKashmirDirectionalSwordContact MakeDirectionalContact()
    {
        FKashmirDirectionalSwordContact Contact;
        Contact.bResolved = true;
        Contact.ActionId = TEXT("Sword.Direct.Right");
        Contact.ActionRequest.ActionId = Contact.ActionId;
        Contact.ActionRequest.Direction = FVector2D::UnitX();
        Contact.ActionRequest.Intensity = 1.0f;
        Contact.CombatDefinition.Delivery = EKashmirDeliveryType::Contact;
        Contact.CombatDefinition.Target = EKashmirTargetType::Enemy;
        Contact.CombatDefinition.Effects.Add(EKashmirResolutionType::Damage);
        Contact.BaseDamage = 24.0f;
        Contact.AttackPowerMultiplier = 1.0f;
        Contact.BaseGuardDamage = 20.0f;
        Contact.TraceHit.Hit.bBlockingHit = true;
        Contact.TraceHit.Hit.ImpactPoint = FVector(100.0f, 0.0f, 100.0f);
        Contact.TraceHit.Hit.ImpactNormal = FVector::ForwardVector;
        Contact.TraceHit.Hit.BoneName = TEXT("spine_03");
        Contact.TraceHit.ContactPointId = TEXT("Weapon_Tip");
        Contact.TraceHit.ContactVelocity = FVector(-800.0f, 0.0f, 0.0f);
        Contact.TraceHit.AttackDirection = FVector::BackwardVector;
        Contact.TraceHit.Speed = 800.0f;
        return Contact;
    }

    FKashmirDirectionalMeleeInput MakeDirectionalInput()
    {
        FKashmirDirectionalMeleeInput Input;
        Input.InstigatorId = TEXT("Player");
        Input.TargetId = TEXT("Enemy_01");
        Input.SourceId = TEXT("Sword_Prototype");
        Input.HitRegionMap = NewObject<UKashmirHitRegionMap>();
        Input.HitRegionMap->BoneToRegion.Add(
            TEXT("spine_03"),
            FGameplayTag::RequestGameplayTag(TEXT("HitRegion.Torso")));
        Input.DefenseInput.BlockState.ForwardDirection = FVector::ForwardVector;
        Input.DefenseInput.BlockState.HalfAngleDegrees = 60.0f;
        Input.DefenseInput.GuardDamageMultiplier = 1.0f;
        Input.DefenseInput.AvailableStamina = 100.0f;
        return Input;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalMeleeResolverMatrixTest,
    "Kashmir.Combat.DirectionalSword.MeleePipeline.Matrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalMeleeResolverMatrixTest::RunTest(
    const FString& Parameters)
{
    const FKashmirDirectionalSwordContact Contact = MakeDirectionalContact();
    FKashmirDirectionalMeleeResolver Resolver;
    FString Reason;

    FKashmirDirectionalMeleeInput Input = MakeDirectionalInput();
    Input.DefenseInput.BlockState.bActive = false;
    FKashmirDirectionalMeleeResult Result;
    TestTrue(TEXT("Unblocked directional contact resolves"),
        Resolver.Resolve(Contact, Input, Result, Reason));
    TestTrue(TEXT("Unblocked contact allows damage"),
        Result.Defense.bDamageAllowed);
    TestEqual(TEXT("Unblocked contact resolves one effect"),
        Result.Defense.HitResult.CombatResult.Effects.Num(), 1);
    TestEqual(TEXT("Authored damage reaches melee pipeline"),
        Result.Defense.HitResult.CombatResult.Effects[0].Magnitude, 24.0f);
    UKashmirCombatantComponent* UnblockedTarget =
        NewObject<UKashmirCombatantComponent>();
    FKashmirCombatantApplicationResult Application;
    TestTrue(TEXT("Unblocked result applies through combatant boundary"),
        UnblockedTarget->ApplyResolvedMelee(Result, Application, Reason));
    TestEqual(TEXT("Unblocked damage changes health once"),
        UnblockedTarget->GetHealthState().Current, 76.0f);

    Input = MakeDirectionalInput();
    Input.DefenseInput.BlockState.bActive = true;
    TestTrue(TEXT("Blocked directional contact resolves"),
        Resolver.Resolve(Contact, Input, Result, Reason));
    TestTrue(TEXT("Front contact is blocked"),
        Result.Defense.DefenseResult.bBlocked);
    TestFalse(TEXT("Block suppresses current health damage"),
        Result.Defense.bDamageAllowed);
    TestTrue(TEXT("Blocked damage effect is marked suppressed"),
        Result.Defense.HitResult.CombatResult.Effects[0].bSuppressed);
    TestEqual(TEXT("Authored guard damage consumes stamina"),
        Result.Defense.DefenseResult.Guard.StaminaAfter, 80.0f);
    UKashmirCombatantComponent* BlockingTarget =
        NewObject<UKashmirCombatantComponent>();
    TestTrue(TEXT("Blocked result applies through combatant boundary"),
        BlockingTarget->ApplyResolvedMelee(Result, Application, Reason));
    TestEqual(TEXT("Suppressed block damage preserves health"),
        BlockingTarget->GetHealthState().Current, 100.0f);
    TestEqual(TEXT("Block applies guard stamina exactly once"),
        BlockingTarget->GetStamina(), 80.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDirectionalMeleeResolverInvalidTest,
    "Kashmir.Combat.DirectionalSword.MeleePipeline.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirDirectionalMeleeResolverInvalidTest::RunTest(
    const FString& Parameters)
{
    FKashmirDirectionalMeleeResolver Resolver;
    FString Reason;
    FKashmirDirectionalMeleeResult Result;
    FKashmirDirectionalSwordContact Contact = MakeDirectionalContact();
    FKashmirDirectionalMeleeInput Input = MakeDirectionalInput();

    Contact.bResolved = false;
    TestFalse(TEXT("Unresolved contact is rejected"),
        Resolver.Resolve(Contact, Input, Result, Reason));

    Contact = MakeDirectionalContact();
    Input.TargetId = NAME_None;
    TestFalse(TEXT("Missing stable target identity is rejected"),
        Resolver.Resolve(Contact, Input, Result, Reason));

    Input = MakeDirectionalInput();
    Input.HitRegionMap = nullptr;
    TestFalse(TEXT("Missing region map is rejected"),
        Resolver.Resolve(Contact, Input, Result, Reason));
    return true;
}

#endif
