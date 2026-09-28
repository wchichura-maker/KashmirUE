#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirMeleeArchetype.h"
#include "Misc/AutomationTest.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMeleeArchetypeMatrixTest,
    "Kashmir.Combat.Archetypes.SharedRuntimeMatrix",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirMeleeArchetypeMatrixTest::RunTest(const FString& Parameters)
{
    const TArray<EKashmirMeleeArchetype> Archetypes = {
        EKashmirMeleeArchetype::OneHandedSword,
        EKashmirMeleeArchetype::TwoHandedSword,
        EKashmirMeleeArchetype::Axe,
        EKashmirMeleeArchetype::Spear,
        EKashmirMeleeArchetype::Unarmed
    };

    FKashmirMeleeArchetypeResolver Resolver;
    TSet<FName> StyleIds;
    FString Reason;
    for (const EKashmirMeleeArchetype Archetype : Archetypes)
    {
        FKashmirMeleeArchetypeDefinition Definition =
            FKashmirMeleeArchetypeResolver::MakeBaseline(Archetype);
        TestTrue(TEXT("Baseline archetype validates"),
            Definition.IsValid(Reason));

        FKashmirMeleeArchetypeResult Result;
        TestTrue(TEXT("Archetype resolves through shared combat values"),
            Resolver.Resolve(Definition, 20.0f, 1.25f, 12.0f,
                Result, Reason));
        TestTrue(TEXT("Archetype result is explicit"), Result.bResolved);
        TestEqual(TEXT("Archetype identity is preserved"),
            Result.Archetype, Archetype);
        TestEqual(TEXT("Untuned baseline preserves health damage"),
            Result.BaseDamage, 20.0f);
        TestEqual(TEXT("Untuned baseline preserves guard damage"),
            Result.BaseGuardDamage, 12.0f);
        TestEqual(TEXT("Shared offensive multiplier is preserved"),
            Result.AttackPowerMultiplier, 1.25f);
        StyleIds.Add(Result.StyleId);

        const bool bExpectedWeapon =
            Archetype != EKashmirMeleeArchetype::Unarmed;
        TestEqual(TEXT("Contact evidence matches equipment semantics"),
            Result.ContactSource == EKashmirContactSourceType::Weapon,
            bExpectedWeapon);
    }
    TestEqual(TEXT("All baseline styles have stable unique ids"),
        StyleIds.Num(), Archetypes.Num());

    FKashmirMeleeArchetypeDefinition Tuned =
        FKashmirMeleeArchetypeResolver::MakeBaseline(
            EKashmirMeleeArchetype::Axe);
    Tuned.DamageMultiplier = 1.5f;
    Tuned.GuardDamageMultiplier = 2.0f;
    FKashmirMeleeArchetypeResult TunedResult;
    TestTrue(TEXT("Data-driven tuning resolves"),
        Resolver.Resolve(Tuned, 20.0f, 1.0f, 12.0f,
            TunedResult, Reason));
    TestEqual(TEXT("Damage tuning is applied exactly once"),
        TunedResult.BaseDamage, 30.0f);
    TestEqual(TEXT("Guard tuning is applied exactly once"),
        TunedResult.BaseGuardDamage, 24.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMeleeArchetypeInvalidTest,
    "Kashmir.Combat.Archetypes.RejectInvalid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter)


bool FKashmirMeleeArchetypeInvalidTest::RunTest(const FString& Parameters)
{
    FKashmirMeleeArchetypeResolver Resolver;
    FKashmirMeleeArchetypeResult Result;
    FString Reason;

    FKashmirMeleeArchetypeDefinition Definition =
        FKashmirMeleeArchetypeResolver::MakeBaseline(
            EKashmirMeleeArchetype::Spear);
    Definition.SourceId = NAME_None;
    TestFalse(TEXT("Missing source identity is rejected"),
        Resolver.Resolve(Definition, 10.0f, 1.0f, 5.0f,
            Result, Reason));

    Definition = FKashmirMeleeArchetypeResolver::MakeBaseline(
        EKashmirMeleeArchetype::Axe);
    const FName DuplicatePoint = Definition.ContactPointIds[0];
    Definition.ContactPointIds.Add(DuplicatePoint);
    TestFalse(TEXT("Duplicate contact point is rejected"),
        Resolver.Resolve(Definition, 10.0f, 1.0f, 5.0f,
            Result, Reason));

    Definition = FKashmirMeleeArchetypeResolver::MakeBaseline(
        EKashmirMeleeArchetype::Unarmed);
    Definition.ContactSource = EKashmirContactSourceType::Weapon;
    TestFalse(TEXT("Unarmed weapon evidence is rejected"),
        Resolver.Resolve(Definition, 10.0f, 1.0f, 5.0f,
            Result, Reason));

    Definition = FKashmirMeleeArchetypeResolver::MakeBaseline(
        EKashmirMeleeArchetype::OneHandedSword);
    TestFalse(TEXT("Negative authored damage is rejected"),
        Resolver.Resolve(Definition, -1.0f, 1.0f, 5.0f,
            Result, Reason));
    return true;
}

#endif
