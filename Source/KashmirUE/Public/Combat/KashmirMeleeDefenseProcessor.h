#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirDefensePipeline.h"
#include "Combat/KashmirMeleeHitProcessor.h"

#include "KashmirMeleeDefenseProcessor.generated.h"


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirMeleeDefenseInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirDefensePipelineInput DefenseInput;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirMeleeDefenseResult
{
    GENERATED_BODY()

    /*
     * Native combat processing result.
     *
     * FKashmirMeleeHitProcessResult is currently
     * a native C++ contract rather than a reflected
     * USTRUCT, so this member intentionally does
     * not use UPROPERTY.
     */
    FKashmirMeleeHitProcessResult HitResult;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirDefensePipelineResult DefenseResult;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bDamageAllowed = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bDamageSuppressedByParry = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bDamageSuppressedByBlock = false;
};


class KASHMIRUE_API FKashmirMeleeDefenseProcessor
{
public:

    bool Resolve(
        const FKashmirMeleeHitProcessResult& HitResult,
        const FKashmirMeleeDefenseInput& Input,
        FKashmirMeleeDefenseResult& OutResult,
        FString& OutReason
    ) const;
};
