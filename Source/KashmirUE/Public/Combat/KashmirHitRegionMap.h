#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KashmirHitRegionMap.generated.h"

/**
 * Maps skeleton-specific bone names to semantic combat regions.
 *
 * Combat systems must consume HitRegion tags rather than depending
 * directly on a particular skeleton's bone naming convention.
 */
UCLASS(BlueprintType)
class KASHMIRUE_API UKashmirHitRegionMap : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category="Hit Regions"
    )
    TMap<FName, FGameplayTag> BoneToRegion;

    UFUNCTION(
        BlueprintPure,
        Category="Combat|Hit Regions"
    )
    bool ResolveRegion(
        FName BoneName,
        FGameplayTag& OutRegion
    ) const;
};