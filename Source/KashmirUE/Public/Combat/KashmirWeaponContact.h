#pragma once

#include "CoreMinimal.h"
#include "KashmirWeaponContact.generated.h"

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirWeaponContactPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    bool IsValid(FString& OutReason) const;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirWeaponContactFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FKashmirWeaponContactPoint> Points;

    bool IsValid(FString& OutReason) const;

    const FKashmirWeaponContactPoint* FindPoint(
        FName PointId
    ) const;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirWeaponSweepSegment
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName PointId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector PreviousPosition = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector CurrentPosition = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Delta = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Distance = 0.0f;
};

class KASHMIRUE_API FKashmirWeaponContactBuilder
{
public:

    static bool BuildSweepSegments(
        const FKashmirWeaponContactFrame& PreviousFrame,
        const FKashmirWeaponContactFrame& CurrentFrame,
        TArray<FKashmirWeaponSweepSegment>& OutSegments,
        FString& OutReason
    );
};