#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "KashmirHurtboxComponent.generated.h"

class UKashmirHurtboxProfile;
class USkeletalMeshComponent;
class UPrimitiveComponent;


UCLASS(
    ClassGroup=(Kashmir),
    meta=(BlueprintSpawnableComponent)
)
class KASHMIRUE_API UKashmirHurtboxComponent :
    public UActorComponent
{
    GENERATED_BODY()

public:

    UKashmirHurtboxComponent();

    bool Build(
        USkeletalMeshComponent* InMesh,
        const UKashmirHurtboxProfile* InProfile,
        FString& OutReason
    );

    void ClearHurtboxes();

    UFUNCTION(BlueprintPure)
    int32 GetHurtboxCount() const;

    bool ResolveHitComponent(
        const UPrimitiveComponent* HitComponent,
        FName& OutHurtboxId,
        FGameplayTag& OutHitRegion
    ) const;

    const TArray<TObjectPtr<UPrimitiveComponent>>&
        GetHurtboxComponents() const
    {
        return HurtboxComponents;
    }

private:

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> BoundMesh;

    UPROPERTY(Transient)
    TObjectPtr<const UKashmirHurtboxProfile> BoundProfile;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UPrimitiveComponent>>
        HurtboxComponents;
};