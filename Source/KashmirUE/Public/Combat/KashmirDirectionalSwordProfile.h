#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataAsset.h"

#include "Combat/KashmirDirectionalSwordResolver.h"
#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirMeleeArchetype.h"
#include "Combat/KashmirSwordPoseResolver.h"
#include "Runtime/KashmirActionRuntime.h"

#include "KashmirDirectionalSwordProfile.generated.h"


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordAuthoredAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionId;

    /** Presentation asset. It does not advance the authoritative timeline. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MontageSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0001"))
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float StartupDuration = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float ActiveDuration = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float RecoveryDuration = 0.30f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCancellable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EKashmirActionPhase> CancelWindows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FKashmirResourceCost> StartCosts;

    /** Presentation-only constraints consumed by Anim Blueprint / Control Rig. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirSwordPoseConfig PoseConfig;

    /** Logical combat delivery/effects authored independently of the montage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirCombatActionDefinition CombatDefinition;

    /** Baseline numeric damage supplied to the melee hit processor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float BaseDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float AttackPowerMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float BaseGuardDamage = 20.0f;

    bool IsValid(FString& OutReason) const;

    FKashmirActionDefinition BuildRuntimeDefinition() const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordActionPlan
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirMeleeArchetypeDefinition ArchetypeDefinition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirDirectionalSwordResult Gesture;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirActionDefinition RuntimeDefinition;

    /** Resolved Technique identity; metadata only, never execution authority. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName TechniqueId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UAnimMontage> Montage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName MontageSection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PlayRate = 1.0f;

    /** Presentation route selected by the resolved Technique, never gameplay authority. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirMovementIntent MovementIntent = EKashmirMovementIntent::Stationary;

    /** Authored displacement request for a future movement executor. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirTechniqueMovementSpec MovementSpec;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirSwordPoseConfig PoseConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirCombatActionDefinition CombatDefinition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BaseDamage = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AttackPowerMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BaseGuardDamage = 0.0f;
};


UCLASS(BlueprintType)
class KASHMIRUE_API UKashmirDirectionalSwordProfile : public UDataAsset
{
    GENERATED_BODY()

public:

    /** Equipment/body semantics; the directional runtime itself is shared. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FKashmirMeleeArchetypeDefinition ArchetypeDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FKashmirDirectionalSwordConfig GestureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FKashmirSwordAuthoredAction> Actions;

    bool ValidateProfile(FString& OutReason) const;

    const FKashmirSwordAuthoredAction* FindAction(FName ActionId) const;

    bool ResolveActionPlan(
        const FKashmirSwordGestureInput& Input,
        FKashmirSwordActionPlan& OutPlan,
        FString& OutReason
    ) const;

    /** Resolves player, AI, replay or network requests through the same data. */
    bool ResolveActionPlan(
        const FKashmirActionRequest& Request,
        FKashmirSwordActionPlan& OutPlan,
        FString& OutReason
    ) const;
};
