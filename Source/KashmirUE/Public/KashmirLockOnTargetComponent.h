#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "KashmirLockOnTargetComponent.generated.h"

UCLASS(ClassGroup=(Kashmir), meta=(BlueprintSpawnableComponent))
class KASHMIRUE_API UKashmirLockOnTargetComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKashmirLockOnTargetComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LockOn")
    bool bCanBeLockedOn = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LockOn")
    float TargetPriority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LockOn")
    FVector LockOnOffset = FVector(0.0f, 0.0f, 80.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="LockOn")
    FGameplayTagContainer TargetTags;

    UFUNCTION(BlueprintPure, Category="LockOn")
    bool CanBeLockedOn() const
    {
        return bCanBeLockedOn;
    }

    UFUNCTION(BlueprintPure, Category="LockOn")
    FVector GetLockOnWorldLocation() const;
};