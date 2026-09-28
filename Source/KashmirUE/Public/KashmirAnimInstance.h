#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Combat/KashmirSwordRigAdapter.h"
#include "Combat/KashmirSwordPoseResolver.h"
#include "KashmirAnimInstance.generated.h"

class AKashmirCharacter;
class UCharacterMovementComponent;

UCLASS()
class KASHMIRUE_API UKashmirAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category="Animation|Combat|Directional Sword")
    FKashmirSwordPoseResult GetSwordPose() const
    {
        return SwordPose;
    }

    UFUNCTION(BlueprintPure, Category="Animation|Combat|Directional Sword")
    FKashmirSwordRigCurveValues GetSwordRigCurves() const
    {
        return SwordRigCurves;
    }

protected:
    UPROPERTY(BlueprintReadOnly, Category="Animation|Character")
    TObjectPtr<AKashmirCharacter> Character;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Movement")
    bool bShouldMove = false;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Movement")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Movement")
    bool bIsDodging = false;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Movement")
    bool bIsWalking = false;

    UPROPERTY(BlueprintReadOnly, Category="Animation|Combat")
    bool bIsLockedOn = false;

    /** Presentation snapshot consumed by ABP_KashmirCharacter / Control Rig. */
    UPROPERTY(BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    FKashmirSwordPoseResult SwordPose;

    /** Scalar bridge consumed by CR_KashmirSword through animation curves. */
    UPROPERTY(BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    FKashmirSwordRigCurveValues SwordRigCurves;

private:
    void UpdateSwordPresentation();
    void PublishSwordRigCurves();

};
