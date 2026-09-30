#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "Contracts/KashmirActionContracts.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Combat/KashmirSwordPoseResolver.h"
#include "Runtime/KashmirActionRuntime.h"

#include "KashmirCombatTechnique.generated.h"


UENUM(BlueprintType)
enum class EKashmirTechniqueSlot : uint8
{
    None,
    TechniqueSlot1,
    TechniqueSlot2,
    TechniqueSlot3,
    TechniqueSlot4,
    TechniqueSlot5
};


UENUM(BlueprintType)
enum class EKashmirTechniqueRequestSource : uint8
{
    Player,
    AI,
    Replay,
    Network
};


UENUM(BlueprintType)
enum class EKashmirAttackDirection : uint8
{
    None,
    LeftToRight,
    RightToLeft,
    HighToLow,
    LowToHigh,
    DiagonalLeftToRight,
    DiagonalRightToLeft,
    Thrust,
    Rotational,
    Radial
};


UENUM(BlueprintType)
enum class EKashmirAttackShape : uint8
{
    Slash,
    Thrust,
    Sweep,
    Impact,
    Rotational,
    Radial
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirTechniqueRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirTechniqueSlot Slot = EKashmirTechniqueSlot::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirTechniqueRequestSource Source =
        EKashmirTechniqueRequestSource::Player;

    /** Optional targeting evidence. It never selects the authored technique. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D DirectionToTarget = FVector2D::UnitX();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer ContextTags;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatTechniqueDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TechniqueId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName WeaponFamily;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TechniqueFamily;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirAttackDirection AttackDirection = EKashmirAttackDirection::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirAttackShape AttackShape = EKashmirAttackShape::Slash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MontageSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0001"))
    float PlayRate = 1.0f;

    /**
     * Optional Sword presentation authored by this Technique.
     * It changes only the procedural pose layered over Base Motion; gameplay
     * timing, trace, hit evidence, damage and combat resolution remain owned
     * by their existing contracts.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOverrideSwordPresentation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        meta=(EditCondition="bOverrideSwordPresentation"))
    FKashmirSwordPoseConfig SwordPresentation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirActionDefinition RuntimeDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirCombatActionDefinition CombatDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
    float BaseDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0"))
    float BaseGuardDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ContactProfileId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MovementProfileId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetingProfileId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer TechniqueTags;

    /** Compatibility hooks only. Progression is awarded from outcomes elsewhere. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> MasteryChannels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ProgressionContributionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DiscoveryMetadataId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UltimateLineageId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
    int32 UltimateStage = 0;

    bool IsValid(FString& OutReason) const;
    FVector2D GetAuthoredDirectionVector() const;
    FKashmirActionRequest BuildActionRequest(float Intensity) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirTechniqueSlotBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirTechniqueSlot Slot = EKashmirTechniqueSlot::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TechniqueId;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirTechniqueActionPlan
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName StyleId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirTechniqueRequest TechniqueRequest;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirCombatTechniqueDefinition Technique;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirActionRequest ActionRequest;
};


/** Data-driven combat grammar shared by player, AI, replay and network intent. */
UCLASS(BlueprintType)
class KASHMIRUE_API UKashmirWeaponCombatStyle : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StyleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName WeaponFamily;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FKashmirCombatTechniqueDefinition> Techniques;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FKashmirTechniqueSlotBinding> SlotBindings;

    bool ValidateStyle(FString& OutReason) const;

    bool ResolveTechnique(
        const FKashmirTechniqueRequest& Request,
        FKashmirTechniqueActionPlan& OutPlan,
        FString& OutReason) const;
};
