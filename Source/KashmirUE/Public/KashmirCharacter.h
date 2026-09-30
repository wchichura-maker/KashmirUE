#pragma once

#include "CoreMinimal.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "GameFramework/Character.h"
#include "KashmirCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UAnimMontage;
class UKashmirMovementConfig;
class UKashmirMovementDeliveryComponent;
class UKashmirTraversalComponent;
class UKashmirSwordPresentationComponent;
class UKashmirWeaponTraceComponent;
class UKashmirCombatantComponent;
class UStaticMeshComponent;
class USceneComponent;
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

    UFUNCTION(BlueprintPure, Category="Animation")
    bool IsLockOnActive() const
    {
        return IsValid(CurrentLockOnTarget);
    }

    UFUNCTION(BlueprintPure, Category="Traversal")
    bool IsTraversing() const;

    UFUNCTION(BlueprintPure, Category="Animation")
    bool IsDodging() const
    {
        return bIsDodging;
    }

    UFUNCTION(BlueprintPure, Category="Animation")
    bool IsWalking() const { return bWalkRequested; }

    UFUNCTION(BlueprintPure, Category="Traversal")
    UKashmirTraversalComponent* GetTraversalComponent() const
    { return TraversalComponent; }

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    UKashmirSwordPresentationComponent* GetSwordPresentationComponent() const
    { return SwordPresentationComponent; }

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    UKashmirDirectionalSwordComponent* GetDirectionalSwordComponent() const
    { return DirectionalSwordComponent; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery")
    UKashmirMovementDeliveryComponent* GetMovementDeliveryComponent() const
    { return MovementDeliveryComponent; }

    UFUNCTION(BlueprintPure, Category="Combat|Weapon")
    UKashmirWeaponTraceComponent* GetWeaponTraceComponent() const
    { return WeaponTraceComponent; }

    UFUNCTION(BlueprintPure, Category="Combat|Weapon")
    UStaticMeshComponent* GetSwordPrototypeMesh() const
    { return SwordPrototypeMesh; }

    UFUNCTION(BlueprintPure, Category="Combat|Weapon")
    USceneComponent* GetSwordTraceBase() const { return SwordTraceBase; }

    UFUNCTION(BlueprintPure, Category="Combat|Weapon")
    USceneComponent* GetSwordTraceMid() const { return SwordTraceMid; }

    UFUNCTION(BlueprintPure, Category="Combat|Weapon")
    USceneComponent* GetSwordTraceTip() const { return SwordTraceTip; }

    UFUNCTION(BlueprintPure, Category="Combat|State")
    UKashmirCombatantComponent* GetCombatantComponent() const
    { return CombatantComponent; }

    /** Logical combat input seam; physical bindings remain data-driven. */
    UFUNCTION(BlueprintCallable, Category="Combat|Technique")
    bool RequestTechniqueSlot(EKashmirTechniqueSlot Slot);

    UFUNCTION(BlueprintCallable, Category="Traversal")
    void RequestTraversalOrJump();

    UFUNCTION(BlueprintCallable, Category="Traversal")
    void StopTraversalOrJump();

protected:
    virtual void BeginPlay() override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void TurnCharacter(const FInputActionValue& Value);
    void BeginWalk();
    void EndWalk();
    void RefreshMovementSpeed();
    void ApplyPlayerMappingContext();
    void RequestTechniqueSlot1();
    void RequestTechniqueSlot2();
    void RequestTechniqueSlot3();
    void RequestTechniqueSlot4();
    void RequestTechniqueSlot5();
    bool TryStartTraversal();

    void BeginMouseTurnCharacter();
    void EndMouseTurnCharacter();
    void BeginLeftMouseCamera();
    void EndLeftMouseCamera();
    void UpdateMouseForwardMovement();
    UFUNCTION()
    void HandleSwordContact(const FKashmirDirectionalSwordContact& Contact);

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement|Gait")
    bool bWalkRequested = false;

    FVector ActiveDodgeDirection = FVector::ZeroVector;

    float DodgeTimeRemaining = 0.0f;

    UFUNCTION(BlueprintPure, Category="Movement|Config")
    float GetConfiguredWalkSpeed() const;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Traversal")
    TObjectPtr<UKashmirTraversalComponent> TraversalComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Directional Sword")
    TObjectPtr<UKashmirSwordPresentationComponent> SwordPresentationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Directional Sword")
    TObjectPtr<UKashmirDirectionalSwordComponent> DirectionalSwordComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Movement Delivery")
    TObjectPtr<UKashmirMovementDeliveryComponent> MovementDeliveryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
    TObjectPtr<UStaticMeshComponent> SwordPrototypeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
    TObjectPtr<USceneComponent> SwordTraceBase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
    TObjectPtr<USceneComponent> SwordTraceMid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
    TObjectPtr<USceneComponent> SwordTraceTip;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Weapon")
    TObjectPtr<UKashmirWeaponTraceComponent> WeaponTraceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|State")
    TObjectPtr<UKashmirCombatantComponent> CombatantComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Technique")
    TObjectPtr<UKashmirWeaponCombatStyle> WeaponCombatStyle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputMappingContext> PlayerMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> WalkAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> TraversalOrJumpAction;

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputAction> LeftMouseCameraAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Technique")
    TObjectPtr<UInputAction> TechniqueSlot1Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Technique")
    TObjectPtr<UInputAction> TechniqueSlot2Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Technique")
    TObjectPtr<UInputAction> TechniqueSlot3Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Technique")
    TObjectPtr<UInputAction> TechniqueSlot4Action;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Technique")
    TObjectPtr<UInputAction> TechniqueSlot5Action;

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

    bool bLeftMouseCamera = false;

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
