#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Traversal/KashmirTraversalTypes.h"
#include "KashmirTraversalComponent.generated.h"

UCLASS(ClassGroup=(Kashmir), meta=(BlueprintSpawnableComponent))
class KASHMIRUE_API UKashmirTraversalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKashmirTraversalComponent();

    UFUNCTION(BlueprintCallable, Category="Traversal")
    FTraversalQueryResult QueryTraversal() const;

    UFUNCTION(BlueprintPure, Category="Traversal")
    ETraversalType ClassifyTraversal(float ObstacleHeight, float ObstacleDepth) const;

    UFUNCTION(BlueprintPure, Category="Traversal")
    bool IsTopSurfaceAcceptable(const FVector& SurfaceNormal) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Query", meta=(ClampMin="1.0"))
    float ForwardQueryDistance = 160.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Classification", meta=(ClampMin="0.0"))
    float MinimumObstacleHeight = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Classification", meta=(ClampMin="0.0"))
    float LowVaultMaximumHeight = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Classification", meta=(ClampMin="0.0"))
    float HighVaultMaximumHeight = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Classification", meta=(ClampMin="0.0"))
    float MantleMaximumHeight = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Classification", meta=(ClampMin="1.0"))
    float MaximumTraversableDepth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Query", meta=(ClampMin="1.0"))
    float TopProbeHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Query", meta=(ClampMin="1.0"))
    float DepthProbeStep = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Clearance", meta=(ClampMin="0.0"))
    float CapsuleClearanceMargin = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Classification", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinimumTopSurfaceUpDot = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Traversal|Debug")
    bool bDrawDebugTraversal = false;

private:
    bool FindForwardObstacle(
        const FVector& Forward,
        float CapsuleRadius,
        float CapsuleHalfHeight,
        FHitResult& OutHit) const;

    bool FindTopSurface(
        const FHitResult& ForwardHit,
        const FVector& Forward,
        float CharacterBaseZ,
        FHitResult& OutTopHit) const;

    bool MeasureObstacleDepth(
        const FHitResult& ForwardHit,
        const FHitResult& TopHit,
        const FVector& Forward,
        float& OutDepth,
        FVector& OutLastTopPoint) const;

    bool HasCapsuleClearance(
        const FVector& TargetLocation,
        float CapsuleRadius,
        float CapsuleHalfHeight,
        const AActor* SurfaceActor) const;

    void DrawQueryDebug(
        const FTraversalQueryResult& Result,
        const FVector& ForwardStart,
        const FVector& ForwardEnd) const;
};
