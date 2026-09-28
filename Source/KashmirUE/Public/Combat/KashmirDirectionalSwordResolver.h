#pragma once

#include "CoreMinimal.h"
#include "Contracts/KashmirActionContracts.h"

#include "KashmirDirectionalSwordResolver.generated.h"


UENUM(BlueprintType)
enum class EKashmirSwordGestureFamily : uint8
{
    None,
    Direct,
    Curved
};


UENUM(BlueprintType)
enum class EKashmirSwordGestureDirection : uint8
{
    Right,
    UpRight,
    Up,
    UpLeft,
    Left,
    DownLeft,
    Down,
    DownRight
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordGestureInput
{
    GENERATED_BODY()

    /** Ordered mouse positions in input-space coordinates. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> Samples;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationSeconds = 0.0f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordActionBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirSwordGestureFamily Family = EKashmirSwordGestureFamily::Direct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirSwordGestureDirection Direction = EKashmirSwordGestureDirection::Right;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionId;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDirectionalSwordConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MinimumDragDistance = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float FullIntensityDistance = 200.0f;

    /** Path excess ratio at which the gesture becomes Curved. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurvedFamilyThreshold = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FKashmirSwordActionBinding> ActionBindings;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDirectionalSwordResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bGestureResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirSwordGestureFamily Family = EKashmirSwordGestureFamily::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirSwordGestureDirection Direction = EKashmirSwordGestureDirection::Right;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector2D DirectionVector = FVector2D::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float DirectDistance = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PathLength = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AverageSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Intensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Curvature = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirActionRequest ActionRequest;
};


class KASHMIRUE_API FKashmirDirectionalSwordResolver
{
public:

    bool Resolve(
        const FKashmirSwordGestureInput& Input,
        const FKashmirDirectionalSwordConfig& Config,
        FKashmirDirectionalSwordResult& OutResult,
        FString& OutReason
    ) const;
};
