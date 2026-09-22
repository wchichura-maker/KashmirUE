#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KashmirMovementConfig.generated.h"

/**
 * Data-driven configuration for canonical character locomotion.
 *
 * These values describe the neutral movement profile.
 * Future systems such as attributes, encumbrance, equipment,
 * buffs, debuffs and terrain will modify the effective result
 * without changing this base configuration.
 */
UCLASS(BlueprintType)
class KASHMIRUE_API UKashmirMovementConfig : public UDataAsset
{
    GENERATED_BODY()

public:

    /** Canonical forward movement speed. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Speed",
        meta=(ClampMin="0.0"))
    float WalkSpeed = 450.0f;

    /** Speed while sprint is successfully resolved. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Speed",
        meta=(ClampMin="0.0"))
    float SprintSpeed = 650.0f;

    /** Lateral movement multiplier relative to forward movement. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Direction",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float StrafeMultiplier = 0.70f;

    /** Backward movement multiplier relative to forward movement. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Direction",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float BackpedalMultiplier = 0.50f;

    /** CharacterMovement acceleration. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Dynamics",
        meta=(ClampMin="0.0"))
    float MaxAcceleration = 2048.0f;

    /** CharacterMovement braking while walking. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Dynamics",
        meta=(ClampMin="0.0"))
    float BrakingDecelerationWalking = 1800.0f;

    /** CharacterMovement ground friction. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Dynamics",
        meta=(ClampMin="0.0"))
    float GroundFriction = 8.0f;

    /** Maximum yaw rotation rate available to locomotion systems. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Rotation",
        meta=(ClampMin="0.0"))
    float RotationRateYaw = 540.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Dodge",
        meta=(ClampMin="0.0"))
    float DodgeSpeed = 950.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Dodge",
        meta=(ClampMin="0.01"))
    float DodgeDuration = 0.22f;
    };