#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KashmirHitEvidence.generated.h"

UENUM(BlueprintType)
enum class EKashmirContactSourceType : uint8
{
    Unknown,
    Weapon,
    Hand,
    Foot,
    Body,
    Projectile,
    Environment
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirHitEvidence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstigatorId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirContactSourceType ContactSource =
        EKashmirContactSourceType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ImpactPoint =
        FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ImpactNormal =
        FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ContactVelocity =
        FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AttackDirection =
        FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelativeSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HitBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag HitRegion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer EvidenceTags;

    bool IsValid(FString& OutReason) const
    {
        if (InstigatorId.IsNone())
        {
            OutReason =
                TEXT("hit evidence requires instigator id");

            return false;
        }

        if (TargetId.IsNone())
        {
            OutReason =
                TEXT("hit evidence requires target id");

            return false;
        }

        if (!FMath::IsFinite(RelativeSpeed) ||
            RelativeSpeed < 0.0f)
        {
            OutReason =
                TEXT("relative speed must be non-negative");

            return false;
        }

        const auto IsFiniteVector =
            [](const FVector& Vector)
        {
            return
                FMath::IsFinite(Vector.X) &&
                FMath::IsFinite(Vector.Y) &&
                FMath::IsFinite(Vector.Z);
        };

        if (!IsFiniteVector(ImpactPoint) ||
            !IsFiniteVector(ImpactNormal) ||
            !IsFiniteVector(ContactVelocity) ||
            !IsFiniteVector(AttackDirection))
        {
            OutReason =
                TEXT("hit evidence vectors must be finite");

            return false;
        }

        return true;
    }
};