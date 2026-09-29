#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirSwordPoseResolver.h"

#include "KashmirSwordRigAdapter.generated.h"


/** Presentation values copied into public Control Rig inputs by the AnimGraph node. */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordRigInputs
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector LeadHandOffset = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector SupportHandOffset = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FRotator AimRotation = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BodyLeanDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float SwordPoseAlpha = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LeftFootLockAlpha = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RightFootLockAlpha = 0.0f;
};


/** Converts the semantic pose result into the Control Rig transport contract. */
class KASHMIRUE_API FKashmirSwordRigAdapter
{
public:
    bool Resolve(
        const FKashmirSwordPoseResult& Pose,
        FKashmirSwordRigInputs& OutInputs,
        FString& OutReason) const;
};
