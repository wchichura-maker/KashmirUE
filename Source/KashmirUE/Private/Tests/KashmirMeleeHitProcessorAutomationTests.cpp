#include "Misc/AutomationTest.h"

#include "Combat/KashmirHitRegionMap.h"
#include "Combat/KashmirMeleeHitProcessor.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirMeleeHitProcessorTest,
    "Kashmir.Combat.MeleeHitProcessor.ContactDamage",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirMeleeHitProcessorTest::RunTest(
    const FString& Parameters)
{
    FKashmirWeaponTraceHit TraceHit;

    TraceHit.Hit.bBlockingHit = true;

    TraceHit.Hit.ImpactPoint =
        FVector(50.0, 0.0, 100.0);

    TraceHit.Hit.ImpactNormal =
        FVector(-1.0, 0.0, 0.0);

    TraceHit.Hit.BoneName =
        TEXT("spine_03");

    TraceHit.ContactPointId =
        TEXT("Tip");

    TraceHit.ContactVelocity =
        FVector(900.0, 0.0, 0.0);

    TraceHit.AttackDirection =
        FVector(1.0, 0.0, 0.0);

    TraceHit.Speed =
        900.0f;

    UKashmirHitRegionMap* RegionMap =
        NewObject<UKashmirHitRegionMap>();

    const FGameplayTag TorsoTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("HitRegion.Torso")
        );

    RegionMap->BoneToRegion.Add(
        TEXT("spine_03"),
        TorsoTag
    );

    FKashmirCombatActionDefinition Definition;

    Definition.Delivery =
        EKashmirDeliveryType::Contact;

    Definition.Target =
        EKashmirTargetType::Enemy;

    Definition.Effects.Add(
        EKashmirResolutionType::Damage
    );

    FKashmirMeleeHitProcessInput Input;

    Input.InstigatorId =
        TEXT("Player");

    Input.TargetId =
        TEXT("Enemy_01");

    Input.SourceId =
        TEXT("Sword_01");

    Input.HitRegionMap =
        RegionMap;

    Input.BaseDamage =
        25.0f;

    Input.AttackPowerMultiplier =
        1.2f;

    Input.RegionMultiplier =
        1.0f;

    FKashmirMeleeHitProcessResult Result;
    FString Reason;

    FKashmirMeleeHitProcessor Processor;

    const bool bProcessed =
        Processor.Process(
            TraceHit,
            Definition,
            Input,
            Result,
            Reason
        );

    TestTrue(
        TEXT("Physical melee hit processes"),
        bProcessed
    );

    TestEqual(
        TEXT("Evidence preserves instigator"),
        Result.Evidence.InstigatorId,
        FName(TEXT("Player"))
    );

    TestEqual(
        TEXT("Evidence preserves target"),
        Result.Evidence.TargetId,
        FName(TEXT("Enemy_01"))
    );

    TestEqual(
        TEXT("Evidence preserves source"),
        Result.Evidence.SourceId,
        FName(TEXT("Sword_01"))
    );

    TestEqual(
        TEXT("Evidence preserves hit bone"),
        Result.Evidence.HitBone,
        FName(TEXT("spine_03"))
    );

    TestTrue(
        TEXT("Evidence resolves torso"),
        Result.Evidence.HitRegion.MatchesTagExact(
            TorsoTag
        )
    );

    TestTrue(
        TEXT("Evidence preserves contact speed"),
        FMath::IsNearlyEqual(
            Result.Evidence.RelativeSpeed,
            900.0f
        )
    );

    TestTrue(
        TEXT("Combat delivery succeeds"),
        Result.CombatResult.bDelivered
    );

    TestEqual(
        TEXT("Combat result has one target"),
        Result.CombatResult.TargetIds.Num(),
        1
    );

    const FGameplayTag DamageTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Effect.Damage")
        );

    TestTrue(
        TEXT("Combat result contains damage effect"),
        Result.CombatResult.EffectTags.HasTagExact(
            DamageTag
        )
    );
    TestEqual(
        TEXT("One detailed effect is produced"),
        Result.CombatResult.Effects.Num(),
        1
    );

    if (Result.CombatResult.Effects.Num() == 1)
    {
        const FKashmirEffectResult& Effect =
            Result.CombatResult.Effects[0];

        TestEqual(
            TEXT("Detailed effect targets enemy"),
            Effect.TargetId,
            FName(TEXT("Enemy_01"))
        );

        TestEqual(
            TEXT("Detailed effect type is damage"),
            Effect.Resolution,
            EKashmirResolutionType::Damage
        );

        TestTrue(
            TEXT("Detailed effect has damage tag"),
            Effect.EffectTag.MatchesTagExact(
                DamageTag
            )
        );

        TestTrue(
            TEXT("Detailed effect preserves torso region"),
            Effect.HitRegion.MatchesTagExact(
                TorsoTag
            )
        );

        TestTrue(
            TEXT("Detailed effect preserves impact direction"),
            Effect.ImpactDirection.Equals(
                FVector(1.0, 0.0, 0.0),
                0.001
            )
        );

        TestTrue(
            TEXT("Detailed effect preserves impact strength"),
            FMath::IsNearlyEqual(
                Effect.ImpactStrength,
                900.0f
            )
        );

        TestTrue(
            TEXT("Damage magnitude is resolved"),
            FMath::IsNearlyEqual(
                Effect.Magnitude,
                30.0f,
                0.001f
            )
        );
    }
    TestTrue(
        TEXT("Processing succeeds without error"),
        Reason.IsEmpty()
    );

    return true;
}

#endif