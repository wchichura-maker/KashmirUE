#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirHitEvidence.h"

#include "KashmirMeleeArchetype.generated.h"


UENUM(BlueprintType)
enum class EKashmirMeleeArchetype : uint8
{
    OneHandedSword,
    TwoHandedSword,
    Axe,
    Spear,
    Unarmed
};


/**
 * Data-only description of a melee style. It deliberately contains no
 * animation or runtime state so every archetype can share ActionRequest,
 * ActionRuntime, contact evidence and defense resolution.
 */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirMeleeArchetypeDefinition
{
    GENERATED_BODY()

    FKashmirMeleeArchetypeDefinition();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirMeleeArchetype Archetype =
        EKashmirMeleeArchetype::OneHandedSword;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StyleId = TEXT("Melee.OneHandedSword");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceId = TEXT("Weapon.MainHand");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirContactSourceType ContactSource =
        EKashmirContactSourceType::Weapon;

    /** Semantic trace points supplied by the equipped mesh or body rig. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> ContactPointIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
    float GuardDamageMultiplier = 1.0f;

    /** Relative reach used by equipment/presentation, not by damage math. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0001"))
    float ReachScale = 1.0f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirMeleeArchetypeResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirMeleeArchetype Archetype =
        EKashmirMeleeArchetype::OneHandedSword;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName StyleId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName SourceId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirContactSourceType ContactSource =
        EKashmirContactSourceType::Weapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BaseDamage = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AttackPowerMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BaseGuardDamage = 0.0f;
};


class KASHMIRUE_API FKashmirMeleeArchetypeResolver
{
public:
    static FKashmirMeleeArchetypeDefinition MakeBaseline(
        EKashmirMeleeArchetype Archetype);

    bool Resolve(
        const FKashmirMeleeArchetypeDefinition& Definition,
        float BaseDamage,
        float AttackPowerMultiplier,
        float BaseGuardDamage,
        FKashmirMeleeArchetypeResult& OutResult,
        FString& OutReason) const;
};
