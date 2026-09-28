#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/KashmirWeaponContact.h"
#include "KashmirWeaponTraceComponent.generated.h"

class USceneComponent;

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirWeaponContactPointBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<USceneComponent> Component;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirWeaponTraceHit
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FHitResult Hit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName ContactPointId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector ContactVelocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector AttackDirection = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Speed = 0.0f;
};

UCLASS(
    ClassGroup=(Kashmir),
    meta=(BlueprintSpawnableComponent)
)
class KASHMIRUE_API UKashmirWeaponTraceComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:

    UKashmirWeaponTraceComponent();

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon Trace")
    void SetTraceSource(
        USceneComponent* InTraceSource
    );

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon Trace")
    void SetContactPointNames(
        const TArray<FName>& InPointNames
    );

    /** Explicit world-space contact components; preferred for equipped weapons. */
    UFUNCTION(BlueprintCallable, Category="Combat|Weapon Trace")
    void SetContactPointBindings(
        const TArray<FKashmirWeaponContactPointBinding>& InBindings
    );

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon Trace")
    void SetIgnoredActor(
        AActor* InIgnoredActor
    );

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon Trace")
    void BeginTraceWindow();

    UFUNCTION(BlueprintCallable, Category="Combat|Weapon Trace")
    void EndTraceWindow();

    bool SampleTrace(
        float DeltaSeconds,
        TArray<FKashmirWeaponTraceHit>& OutHits,
        FString& OutReason
    );

    UFUNCTION(BlueprintPure, Category="Combat|Weapon Trace")
    bool IsTraceWindowActive() const
    {
        return bTraceWindowActive;
    }

protected:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon Trace")
    float TraceRadius = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Weapon Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel1;

private:

    bool BuildCurrentFrame(
        FKashmirWeaponContactFrame& OutFrame,
        FString& OutReason
    ) const;

    UPROPERTY()
    TObjectPtr<USceneComponent> TraceSource;

    UPROPERTY()
    TArray<FName> ContactPointNames;

    UPROPERTY()
    TArray<FKashmirWeaponContactPointBinding> ContactPointBindings;

    UPROPERTY()
    TObjectPtr<AActor> IgnoredActor;

    FKashmirWeaponContactFrame PreviousFrame;

    bool bHasPreviousFrame = false;
    bool bTraceWindowActive = false;

    TSet<TWeakObjectPtr<AActor>> HitActorsThisWindow;
};
