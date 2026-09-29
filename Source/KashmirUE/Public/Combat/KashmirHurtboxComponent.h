#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "KashmirHurtboxComponent.generated.h"

class UKashmirHurtboxProfile;
class USkeletalMeshComponent;
class UPrimitiveComponent;
struct FKashmirHurtboxDefinition;


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

    virtual void BeginPlay() override;

    bool Build(
        USkeletalMeshComponent* InMesh,
        const UKashmirHurtboxProfile* InProfile,
        FString& OutReason
    );

    /** Builds the runtime hurtboxes from HurtboxProfile and TargetMesh. */
    UFUNCTION(BlueprintCallable, Category="Combat|Hurtbox")
    bool BuildFromProfile(
        FString& OutReason
    );

    void ClearHurtboxes();

    UFUNCTION(BlueprintPure, Category="Combat|Hurtbox")
    int32 GetHurtboxCount() const;

    UFUNCTION(BlueprintCallable, Category="Combat|Hurtbox")
    bool ResolveHitComponent(
        const UPrimitiveComponent* HitComponent,
        FName& OutHurtboxId,
        FGameplayTag& OutHitRegion
    ) const;

    /**
     * Semantic definition that owns the given hit component.
     *
     * Dedicated hurtboxes are the authoritative combat anatomy, so the
     * damage path needs the defining bone and region, not just an index.
     */
    const FKashmirHurtboxDefinition* FindDefinitionForComponent(
        const UPrimitiveComponent* HitComponent
    ) const;

    UFUNCTION(BlueprintPure, Category="Combat|Hurtbox")
    TArray<UPrimitiveComponent*> GetHurtboxPrimitives() const;

    const TArray<TObjectPtr<UPrimitiveComponent>>&
        GetHurtboxComponents() const
    {
        return HurtboxComponents;
    }

protected:

    /** Authoritative combat anatomy generated for this actor. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hurtbox")
    TObjectPtr<UKashmirHurtboxProfile> HurtboxProfile;

    /**
     * Skeletal mesh the hurtboxes attach to.
     *
     * Falls back to the owner's first SkeletalMeshComponent when unset.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hurtbox")
    TObjectPtr<USkeletalMeshComponent> TargetMesh;

private:

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> BoundMesh;

    UPROPERTY(Transient)
    TObjectPtr<const UKashmirHurtboxProfile> BoundProfile;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UPrimitiveComponent>>
        HurtboxComponents;
};
