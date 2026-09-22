#include "KashmirLockOnTargetComponent.h"
#include "GameFramework/Actor.h"

UKashmirLockOnTargetComponent::UKashmirLockOnTargetComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FVector UKashmirLockOnTargetComponent::GetLockOnWorldLocation() const
{
    const AActor* Owner = GetOwner();

    if (Owner == nullptr)
    {
        return FVector::ZeroVector;
    }

    return Owner->GetActorLocation() + LockOnOffset;
}