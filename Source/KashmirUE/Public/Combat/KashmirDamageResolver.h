#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KashmirDamageResolver.generated.h"

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDamageInput
{
    GENERATED_BODY()

    /** Base numeric damage supplied by the attack/weapon definition. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 0.0f;

    /** Offensive scaling resolved before target mitigation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackPowerMultiplier = 1.0f;

    /** Semantic body-region scaling. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RegionMultiplier = 1.0f;

    /** Semantic region reached by the physical hit. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag HitRegion;

    bool IsValid(FString& OutReason) const;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDamageResult
{
    GENERATED_BODY()

    /** Damage before regional scaling. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RawDamage = 0.0f;

    /** Final resolved damage before mitigation/application. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float FinalDamage = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FGameplayTag HitRegion;
};

class KASHMIRUE_API FKashmirDamageResolver
{
public:

    bool Resolve(
        const FKashmirDamageInput& Input,
        FKashmirDamageResult& OutResult,
        FString& OutReason
    ) const;
};