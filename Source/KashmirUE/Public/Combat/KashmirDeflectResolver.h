#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirHitEvidence.h"
#include "Combat/KashmirParryResolver.h"

#include "KashmirDeflectResolver.generated.h"


UENUM(BlueprintType)
enum class EKashmirDeflectOutcome : uint8
{
    NoDeflect,
    Deflect
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDeflectInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirParryResult ParryResult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float StrengthMultiplier = 1.0f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDeflectResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bDeflected = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirDeflectOutcome Outcome = EKashmirDeflectOutcome::NoDeflect;

    /** Direction from the defender back toward the attack origin. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector ReactionDirection = FVector::ZeroVector;

    /** Physical reaction intensity, independent of damage magnitude. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PhysicalIntensity = 0.0f;
};


class KASHMIRUE_API FKashmirDeflectResolver
{
public:
    bool Resolve(
        const FKashmirHitEvidence& Evidence,
        const FKashmirDeflectInput& Input,
        FKashmirDeflectResult& OutResult,
        FString& OutReason
    ) const;
};
