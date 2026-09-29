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
    FKashmirSwordRigInputs GetSwordRigInputs() const;

    /** Debug/test-only presentation override. Shipping builds always remain in normal mode. */
    UFUNCTION(BlueprintCallable, Category="Animation|Debug|Directional Sword", meta=(DevelopmentOnly))
    void SetForceNeutralSwordRigInputs(bool bForceNeutral);

    UFUNCTION(BlueprintPure, Category="Animation|Debug|Directional Sword", meta=(DevelopmentOnly))
    bool IsForceNeutralSwordRigInputsEnabled() const;

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

    /** Stable game-thread snapshot copied into CR_KashmirSword by exposed AnimGraph inputs. */
    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordLeadHandOffsetX = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordLeadHandOffsetY = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordLeadHandOffsetZ = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordSupportHandOffsetX = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordSupportHandOffsetY = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordSupportHandOffsetZ = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordAimPitch = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordAimYaw = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordAimRoll = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordBodyLean = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordPoseAlpha = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordLeftFootLockAlpha = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Animation|Combat|Directional Sword")
    float SwordRightFootLockAlpha = 0.0f;

private:
    /** Transient presentation-only A/B lever; never consulted by gameplay systems. */
    UPROPERTY(Transient)
    bool bForceNeutralSwordRigInputs = false;

    void UpdateSwordPresentation();
    void ResolveSwordRigInputsFromCurrentPose();
    void ApplySwordRigInputs(const FKashmirSwordRigInputs& Inputs);
    void ResetSwordRigInputs();

};
