#pragma once

#include "CoreMinimal.h"
#include "KashmirTraversalTypes.generated.h"

UENUM(BlueprintType)
enum class ETraversalType : uint8
{
    None,
    Climb,
    LedgeGrab
};

UENUM(BlueprintType)
enum class ETraversalExecutionState : uint8
{
    Idle,
    Preparing,
    Traversing,
    Recovering
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FTraversalQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    ETraversalType Type = ETraversalType::None;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    FVector SurfaceNormal = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    float ObstacleHeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    float ObstacleDepth = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    bool bHasDestinationClearance = false;

    UPROPERTY(BlueprintReadOnly, Category="Traversal")
    TObjectPtr<AActor> SurfaceActor = nullptr;
};
