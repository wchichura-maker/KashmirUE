#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirSwordPoseResolver.h"

#include "KashmirSwordRigAdapter.generated.h"


namespace KashmirSwordRigCurves
{
    KASHMIRUE_API extern const FName LeadHandOffsetX;
    KASHMIRUE_API extern const FName LeadHandOffsetY;
    KASHMIRUE_API extern const FName LeadHandOffsetZ;
    KASHMIRUE_API extern const FName SupportHandOffsetX;
    KASHMIRUE_API extern const FName SupportHandOffsetY;
    KASHMIRUE_API extern const FName SupportHandOffsetZ;
    KASHMIRUE_API extern const FName AimPitch;
    KASHMIRUE_API extern const FName AimYaw;
    KASHMIRUE_API extern const FName AimRoll;
    KASHMIRUE_API extern const FName BodyLean;
    KASHMIRUE_API extern const FName SwordPoseAlpha;
    KASHMIRUE_API extern const FName LeftFootLockAlpha;
    KASHMIRUE_API extern const FName RightFootLockAlpha;
}


/** Scalar presentation contract transported through animation curves. */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordRigCurveValues
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
        FKashmirSwordRigCurveValues& OutCurves,
        FString& OutReason) const;
};
