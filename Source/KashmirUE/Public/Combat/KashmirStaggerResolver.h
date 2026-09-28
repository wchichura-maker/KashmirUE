#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirDeflectResolver.h"
#include "Combat/KashmirGuardResolver.h"

#include "KashmirStaggerResolver.generated.h"


UENUM(BlueprintType)
enum class EKashmirStaggerCause : uint8
{
    None,
    Deflect,
    GuardBreak
};


UENUM(BlueprintType)
enum class EKashmirStaggerTarget : uint8
{
    None,
    Attacker,
    Defender
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirStaggerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MinimumDeflectIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float DeflectDurationSeconds = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float GuardBreakDurationSeconds = 0.60f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirStaggerInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirDeflectResult Deflect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirGuardResult Guard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirStaggerConfig Config;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirStaggerResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bStaggered = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirStaggerCause Cause = EKashmirStaggerCause::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirStaggerTarget Target = EKashmirStaggerTarget::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float DurationSeconds = 0.0f;

    /** Physical cue only; it is not damage or persistent state. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PhysicalIntensity = 0.0f;
};


class KASHMIRUE_API FKashmirStaggerResolver
{
public:

    bool Resolve(
        const FKashmirStaggerInput& Input,
        FKashmirStaggerResult& OutResult,
        FString& OutReason
    ) const;
};
