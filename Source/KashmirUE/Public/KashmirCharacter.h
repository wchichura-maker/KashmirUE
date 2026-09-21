#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KashmirCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class KASHMIRUE_API AKashmirCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AKashmirCharacter();
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    void BeginMouseTurnCharacter();
    void EndMouseTurnCharacter();

    void RequestDodge();
    void ToggleLockOn();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputMappingContext> PlayerMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> DodgeAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> LockOnAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> MouseTurnCharacterAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    bool bDodgeRequested = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    bool bLockOnRequested = false;

    bool bMouseTurnCharacter = false;
};
