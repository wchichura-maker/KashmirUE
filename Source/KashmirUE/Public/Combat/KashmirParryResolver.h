#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirDefenseResolver.h"
#include "Combat/KashmirHitEvidence.h"

#include "KashmirParryResolver.generated.h"


UENUM(BlueprintType)
enum class EKashmirParryTiming : uint8
{
    Inactive,
    TooEarly,
    ExactStart,
    InsideWindow,
    ExactEnd,
    TooLate
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirParryWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTimeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTimeSeconds = 0.0f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirParryState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActive = false;

    /** Time supplied by gameplay since parry activation, independent of animation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ElapsedTimeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirParryWindow Window;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirParryResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bParried = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirParryTiming Timing = EKashmirParryTiming::Inactive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float ElapsedTimeSeconds = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Alignment = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RequiredAlignment = 1.0f;
};


class KASHMIRUE_API FKashmirParryResolver
{
public:

    bool Resolve(
        const FKashmirHitEvidence& Evidence,
        const FKashmirBlockState& DefenseFacing,
        const FKashmirParryState& ParryState,
        FKashmirParryResult& OutResult,
        FString& OutReason
    ) const;
};
