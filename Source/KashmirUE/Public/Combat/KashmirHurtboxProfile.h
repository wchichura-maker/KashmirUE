#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KashmirHurtboxProfile.generated.h"

UENUM(BlueprintType)
enum class EKashmirHurtboxShape : uint8
{
    Box,
    Capsule,
    Sphere
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirHurtboxDefinition
{
    GENERATED_BODY()

    /** Stable semantic id inside this profile. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Id;

    /** Skeleton bone that owns/follows this hurtbox. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName BoneName;

    /** Gameplay-facing anatomical region. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag HitRegion;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EKashmirHurtboxShape Shape =
        EKashmirHurtboxShape::Capsule;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector LocalOffset =
        FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FRotator LocalRotation =
        FRotator::ZeroRotator;

    /**
     * Used only when Shape == Box.
     * Represents Unreal box half extents.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector BoxHalfExtent =
        FVector(10.0f);

    /**
     * Used by Capsule and Sphere.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Radius =
        10.0f;

    /**
     * Used only by Capsule.
     * Unreal capsule half-height.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float HalfHeight =
        20.0f;

    bool IsValid(
        FString& OutReason
    ) const;
};


UCLASS(BlueprintType)
class KASHMIRUE_API UKashmirHurtboxProfile :
    public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FKashmirHurtboxDefinition> Hurtboxes;

    bool ValidateProfile(
        FString& OutReason
    ) const;

    const FKashmirHurtboxDefinition* FindById(
        FName Id
    ) const;
};