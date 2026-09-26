#pragma once

#include "CoreMinimal.h"
#include "Combat/KashmirHitEvidence.h"

#include "KashmirDefenseResolver.generated.h"


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirBlockState
{
    GENERATED_BODY()

    /** Whether the defender is currently guarding. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActive = false;

    /** Defender's world-space facing direction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ForwardDirection =
        FVector::ForwardVector;

    /**
     * Half-angle of the frontal block cone.
     *
     * 60 degrees means a total 120 degree
     * defensive arc.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HalfAngleDegrees = 60.0f;

    bool IsValid(
        FString& OutReason
    ) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDefenseResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bBlocked = false;

    /**
     * Direction from defender toward
     * the source of the incoming attack.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector IncomingSourceDirection =
        FVector::ZeroVector;

    /**
     * Dot product between defender forward
     * and incoming source direction.
     *
     * 1  = directly in front
     * 0  = exactly to the side
     * -1 = directly behind
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Alignment = 0.0f;

    /**
     * Minimum alignment required by
     * the configured block half-angle.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RequiredAlignment = 1.0f;
};


class KASHMIRUE_API FKashmirDefenseResolver
{
public:

    bool ResolveBlock(
        const FKashmirHitEvidence& Evidence,
        const FKashmirBlockState& BlockState,
        FKashmirDefenseResult& OutResult,
        FString& OutReason
    ) const;
};