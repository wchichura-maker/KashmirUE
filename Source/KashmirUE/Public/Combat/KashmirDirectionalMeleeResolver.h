#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirMeleeDefenseProcessor.h"

#include "KashmirDirectionalMeleeResolver.generated.h"


class UKashmirHitRegionMap;


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDirectionalMeleeInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InstigatorId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TargetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceId = TEXT("Sword");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UKashmirHitRegionMap> HitRegionMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer EvidenceTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float RegionMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirDefensePipelineInput DefenseInput;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDirectionalMeleeResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirMeleeDefenseResult Defense;
};


/** Bridges a directional sword contact into the existing melee/defense pipeline. */
class KASHMIRUE_API FKashmirDirectionalMeleeResolver
{
public:
    bool Resolve(
        const FKashmirDirectionalSwordContact& Contact,
        const FKashmirDirectionalMeleeInput& Input,
        FKashmirDirectionalMeleeResult& OutResult,
        FString& OutReason) const;
};
