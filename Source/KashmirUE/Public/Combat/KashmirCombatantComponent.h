#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Combat/KashmirDefensePipeline.h"
#include "Combat/KashmirDirectionalMeleeResolver.h"
#include "Combat/KashmirEffectApplier.h"

#include "KashmirCombatantComponent.generated.h"


class UKashmirHitRegionMap;


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatantApplicationResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bApplied = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FKashmirEffectApplicationResult> Effects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float StaminaBefore = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float StaminaAfter = 0.0f;
};


/** Minimal local combat state adapter. GAS can replace this boundary later. */
UCLASS(ClassGroup=(Kashmir), meta=(BlueprintSpawnableComponent))
class KASHMIRUE_API UKashmirCombatantComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKashmirCombatantComponent();

    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Combat|Identity")
    void SetEntityId(FName InEntityId) { EntityId = InEntityId; }

    UFUNCTION(BlueprintPure, Category="Combat|Identity")
    FName GetEntityId() const { return EntityId; }

    UFUNCTION(BlueprintCallable, Category="Combat|Hit Regions")
    void SetHitRegionMap(UKashmirHitRegionMap* InMap) { HitRegionMap = InMap; }

    UFUNCTION(BlueprintPure, Category="Combat|Hit Regions")
    UKashmirHitRegionMap* GetHitRegionMap() const { return HitRegionMap; }

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void BeginBlock() { BlockState.bActive = true; }

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void EndBlock() { BlockState.bActive = false; }

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void BeginParry();

    UFUNCTION(BlueprintCallable, Category="Combat|Defense")
    void EndParry() { ParryState.bActive = false; }

    FKashmirDefensePipelineInput BuildDefenseInput() const;

    bool ApplyResolvedMelee(
        const FKashmirDirectionalMeleeResult& Result,
        FKashmirCombatantApplicationResult& OutApplication,
        FString& OutReason);

    UFUNCTION(BlueprintPure, Category="Combat|State")
    FKashmirHealthState GetHealthState() const { return Health; }

    UFUNCTION(BlueprintPure, Category="Combat|State")
    float GetStamina() const { return Stamina; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Identity")
    FName EntityId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Hit Regions")
    TObjectPtr<UKashmirHitRegionMap> HitRegionMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|State")
    FKashmirHealthState Health;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|State", meta=(ClampMin="0.0"))
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Defense")
    FKashmirBlockState BlockState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Defense")
    FKashmirParryState ParryState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Defense", meta=(ClampMin="0.0"))
    float GuardDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Defense", meta=(ClampMin="0.0"))
    float DeflectStrengthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Defense")
    FKashmirStaggerConfig StaggerConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Defense")
    FKashmirPhysicalReactionConfig PhysicalReactionConfig;
};
