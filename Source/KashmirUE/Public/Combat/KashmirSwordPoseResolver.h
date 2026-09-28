#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirDirectionalSwordResolver.h"
#include "Runtime/KashmirActionRuntime.h"

#include "KashmirSwordPoseResolver.generated.h"


/**
 * Authored presentation limits consumed by an Animation Blueprint or Control Rig.
 * These values never change the authoritative action timeline or combat result.
 */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordPoseConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LeadHandOffsetAtFullIntensity = FVector(12.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SupportHandOffsetAtFullIntensity = FVector(6.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MaximumHandOffset = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AimYawAtFullIntensity = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AimPitchAtFullIntensity = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MaximumAimYaw = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MaximumAimPitch = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MaximumWeaponRoll = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MaximumBodyLean = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartupFootLockAlpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActiveFootLockAlpha = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RecoveryFootLockAlpha = 1.0f;

    bool IsValid(FString& OutReason) const;
};


/** Presentation-only pose parameters ready for an Anim Blueprint / Control Rig. */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordPoseResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bEnabled = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PhaseAlpha = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector LeadHandOffset = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector SupportHandOffset = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FRotator AimRotation = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BodyLeanDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float LeftFootLockAlpha = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RightFootLockAlpha = 0.0f;
};


class KASHMIRUE_API FKashmirSwordPoseResolver
{
public:

    bool Resolve(
        const FKashmirDirectionalSwordResult& Gesture,
        const FKashmirActionDefinition& Definition,
        const FKashmirSwordPoseConfig& Config,
        const FKashmirActionRuntimeState& RuntimeState,
        FKashmirSwordPoseResult& OutResult,
        FString& OutReason
    ) const;
};
