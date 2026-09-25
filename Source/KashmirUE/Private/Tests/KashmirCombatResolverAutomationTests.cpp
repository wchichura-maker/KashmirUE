#include "Misc/AutomationTest.h"

#include "Combat/KashmirCombatResolver.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirCombatResolverContactDamageTest,
    "Kashmir.Combat.Resolver.ContactDamage",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirCombatResolverContactDamageTest::RunTest(
    const FString& Parameters)
{
    FKashmirCombatResolver Resolver;

    FKashmirCombatActionDefinition Definition;
    Definition.Delivery =
        EKashmirDeliveryType::Contact;
    Definition.Target =
        EKashmirTargetType::Enemy;
    Definition.Effects.Add(
        EKashmirResolutionType::Damage
    );

    FKashmirCombatResolveContext Context;
    Context.InstigatorId =
        TEXT("Player");

    Context.CandidateTargetIds.Add(
        TEXT("Enemy_01")
    );

    FKashmirCombatResult Result;
    FString Reason;

    const bool bResolved =
        Resolver.Resolve(
            Definition,
            Context,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Contact damage resolves"),
        bResolved
    );

    TestTrue(
        TEXT("Delivery succeeds"),
        Result.bDelivered
    );

    TestEqual(
        TEXT("Exactly one target is resolved"),
        Result.TargetIds.Num(),
        1
    );

    TestEqual(
        TEXT("Enemy target is preserved"),
        Result.TargetIds[0],
        FName(TEXT("Enemy_01"))
    );

    const FGameplayTag DamageTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Effect.Damage")
        );

    TestTrue(
        TEXT("Damage effect is present"),
        Result.EffectTags.HasTagExact(
            DamageTag
        )
    );

    TestTrue(
        TEXT("Successful resolution has no error"),
        Reason.IsEmpty()
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirCombatResolverSelfHealTest,
    "Kashmir.Combat.Resolver.SelfHeal",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirCombatResolverSelfHealTest::RunTest(
    const FString& Parameters)
{
    FKashmirCombatResolver Resolver;

    FKashmirCombatActionDefinition Definition;
    Definition.Delivery =
        EKashmirDeliveryType::Self;
    Definition.Target =
        EKashmirTargetType::Self;
    Definition.Effects.Add(
        EKashmirResolutionType::Heal
    );

    FKashmirCombatResolveContext Context;
    Context.InstigatorId =
        TEXT("Player");

    FKashmirCombatResult Result;
    FString Reason;

    const bool bResolved =
        Resolver.Resolve(
            Definition,
            Context,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Self heal resolves"),
        bResolved
    );

    TestTrue(
        TEXT("Self delivery succeeds"),
        Result.bDelivered
    );

    TestEqual(
        TEXT("Self heal resolves one target"),
        Result.TargetIds.Num(),
        1
    );

    TestEqual(
        TEXT("Instigator becomes self target"),
        Result.TargetIds[0],
        FName(TEXT("Player"))
    );

    const FGameplayTag HealTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Effect.Heal")
        );

    TestTrue(
        TEXT("Heal effect is present"),
        Result.EffectTags.HasTagExact(
            HealTag
        )
    );

    TestTrue(
        TEXT("Successful resolution has no error"),
        Reason.IsEmpty()
    );

    return true;
}

#endif