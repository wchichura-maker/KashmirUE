#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

};