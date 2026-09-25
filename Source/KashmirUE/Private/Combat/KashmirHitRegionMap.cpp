#include "Combat/KashmirHitRegionMap.h"

bool UKashmirHitRegionMap::ResolveRegion(
    const FName BoneName,
    FGameplayTag& OutRegion) const
{
    OutRegion = FGameplayTag();

    if (BoneName.IsNone())
    {
        return false;
    }

    const FGameplayTag* Region =
        BoneToRegion.Find(BoneName);

    if (Region == nullptr ||
        !Region->IsValid())
    {
        return false;
    }

    OutRegion = *Region;

    return true;
}