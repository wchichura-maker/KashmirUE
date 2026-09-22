#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KashmirCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UKashmirMovementConfig;
class USpringArmComponent;
class UKashmirLockOnTargetComponent;
struct FInputActionValue;

UCLASS()
class KASHMIRUE_API AKashmirCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AKashmirCharacter();
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void TurnCharacter(const FInputActionValue& Value);

    void BeginMouseTurnCharacter();
    void EndMouseTurnCharacter();

    void RequestDodge();
    void ToggleLockOn();
    void UpdateLockOn(float DeltaSeconds);
    void HandleTurnCompleted(const FInputActionValue& Value);

    TArray<UKashmirLockOnTargetComponent*> FindValidLockOnTargets() const;

    UKashmirLockOnTargetComponent* FindNextLockOnTarget() const;
    void ClearLockOnTarget();

    void ApplyMovementConfig();
    
    void RecenterCameraToCharacter();

    void HandleMoveCompleted(const FInputActionValue& Value);

    void UpdateDodge(float DeltaSeconds);
    FVector CalculateDodgeDirection() const;

    bool bMovementStoppedSinceLastInput = true;
    bool bCameraRecentering = false;
    
    FVector2D LastMoveInput2D = FVector2D::ZeroVector;

    bool bIsDodging = false;

    FVector ActiveDodgeDirection = FVector::ZeroVector;

    float DodgeTimeRemaining = 0.0f;

    UFUNCTION(BlueprintPure, Category="Movement|Config")
    float GetConfiguredWalkSpeed() const;

    UFUNCTION(BlueprintPure, Category="Movement|Config")
    float GetConfiguredSprintSpeed() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|Distance",
        meta=(ClampMin="0.0"))
    float LockOnAcquireDistance = 2000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|Distance",
        meta=(ClampMin="0.0"))
    float LockOnBreakDistance = 2400.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn",
        meta=(ClampMin="-1.0", ClampMax="1.0"))
    float LockOnMinCameraDot = 0.35f;
    float LockOnOrbitRadius = 0.0f;
    float LockOnLostSightTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LockOn")
    TObjectPtr<AActor> CurrentLockOnTarget;

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
    TObjectPtr<UInputAction> TurnCharacterAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> DodgeAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> LockOnAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> MouseTurnCharacterAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Config")
    TObjectPtr<UKashmirMovementConfig> MovementConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Recenter",
        meta=(ClampMin="0.1"))
    float CameraRecenterSpeed = 3.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    bool bDodgeRequested = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Intent")
    bool bLockOnRequested = false;

    bool bMouseTurnCharacter = false;

    bool bManualTurnActive = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|Rotation",
        meta=(ClampMin="0.1"))
    float LockOnBodyRotationSpeed = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|Rotation",
        meta=(ClampMin="0.1"))
    float LockOnCameraRotationSpeed = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|Movement",
        meta=(ClampMin="1.0"))
    float LockOnOrbitCorrectionRange = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|Movement",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float LockOnOrbitMaxCorrection = 0.35f;

    bool HasLineOfSightToLockOnTarget(
        const UKashmirLockOnTargetComponent* TargetComponent) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="LockOn|LineOfSight",
        meta=(ClampMin="0.0"))
    float LockOnLineOfSightGraceTime = 0.75f;
};
