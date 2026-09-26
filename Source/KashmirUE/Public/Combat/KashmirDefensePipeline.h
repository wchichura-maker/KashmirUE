#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirDefenseResolver.h"
#include "Combat/KashmirGuardResolver.h"
#include "Combat/KashmirHitEvidence.h"
#include "Combat/KashmirParryResolver.h"

#include "KashmirDefensePipeline.generated.h"


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDefensePipelineInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirBlockState BlockState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirParryState ParryState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseGuardDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GuardDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AvailableStamina = 0.0f;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDefensePipelineResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirDefenseResult Defense;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirGuardResult Guard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirParryResult Parry;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bParried = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bBlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bGuardBroken = false;
};


class KASHMIRUE_API FKashmirDefensePipeline
{
public:

    bool Resolve(
        const FKashmirHitEvidence& Evidence,
        const FKashmirDefensePipelineInput& Input,
        FKashmirDefensePipelineResult& OutResult,
        FString& OutReason
    ) const;
};
