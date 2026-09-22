#include "KashmirCharacter.h"
#include "KashmirMovementConfig.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "KashmirLockOnTargetComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

AKashmirCharacter::AKashmirCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    bUseControllerRotationPitch = false; bUseControllerRotationYaw = false; bUseControllerRotationRoll = false;
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = false;
    Movement->bUseControllerDesiredRotation = false;
    Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    Movement->MaxWalkSpeed = 450.0f;
    Movement->BrakingDecelerationWalking = 1800.0f;
    Movement->MaxAcceleration = 2048.0f;
    Movement->GroundFriction = 8.0f;
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 350.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 60.0f);
    CameraBoom->bUsePawnControlRotation = true;

    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.0f;

    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.0f;

    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 15.0f;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); FollowCamera->bUsePawnControlRotation = false;
}
void AKashmirCharacter::BeginPlay()
{
    Super::BeginPlay();
    ApplyMovementConfig();
    const APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC == nullptr || PlayerMappingContext == nullptr) { return; }
    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)) { Subsystem->AddMappingContext(PlayerMappingContext, 0); }
}
void AKashmirCharacter::SetupPlayerInputComponent(UInputComponent* Component)
{
    Super::SetupPlayerInputComponent(Component);
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(Component);
    if (Input == nullptr) { UE_LOG(LogTemp, Error, TEXT("KashmirCharacter requires Enhanced Input.")); return; }
    if (MoveAction)
    {
        Input->BindAction(
            MoveAction,
            ETriggerEvent::Triggered,
            this,
            &AKashmirCharacter::Move
        );

        Input->BindAction(
            MoveAction,
            ETriggerEvent::Completed,
            this,
            &AKashmirCharacter::HandleMoveCompleted
        );

        Input->BindAction(
            MoveAction,
            ETriggerEvent::Canceled,
            this,
            &AKashmirCharacter::HandleMoveCompleted
        );
    }
    if (LookAction) Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKashmirCharacter::Look);
    if (TurnCharacterAction)
    {
        Input->BindAction(
            TurnCharacterAction,
            ETriggerEvent::Triggered,
            this,
            &AKashmirCharacter::TurnCharacter
        );
    }
    if (DodgeAction) Input->BindAction(DodgeAction, ETriggerEvent::Started, this, &AKashmirCharacter::RequestDodge);
    if (LockOnAction) Input->BindAction(LockOnAction, ETriggerEvent::Started, this, &AKashmirCharacter::ToggleLockOn);
    if (MouseTurnCharacterAction)
    {
        Input->BindAction(
            MouseTurnCharacterAction,
            ETriggerEvent::Started,
            this,
            &AKashmirCharacter::BeginMouseTurnCharacter
        );

        Input->BindAction(
            MouseTurnCharacterAction,
            ETriggerEvent::Completed,
            this,
            &AKashmirCharacter::EndMouseTurnCharacter
        );
    }
}

void AKashmirCharacter::HandleMoveCompleted(
    const FInputActionValue& Value)
{
    bMovementStoppedSinceLastInput = true;
}

void AKashmirCharacter::TurnCharacter(const FInputActionValue& Value)
{
    if (Controller == nullptr)
    {
        return;
    }

    const float Input = Value.Get<float>();

    if (FMath::IsNearlyZero(Input))
    {
        return;
    }

    const float RotationSpeed =
        MovementConfig
            ? MovementConfig->RotationRateYaw
            : 120.0f;

    const float DeltaYaw =
        Input * RotationSpeed * GetWorld()->GetDeltaSeconds();

    AddActorLocalRotation(
        FRotator(0.0f, DeltaYaw, 0.0f)
    );

    FRotator ControlRotation =
        Controller->GetControlRotation();

    ControlRotation.Yaw += DeltaYaw;

    Controller->SetControlRotation(ControlRotation);
}

void AKashmirCharacter::RecenterCameraToCharacter()
{
    if (Controller == nullptr)
    {
        return;
    }

    FRotator ControlRotation =
        Controller->GetControlRotation();

    ControlRotation.Yaw =
        GetActorRotation().Yaw;

    Controller->SetControlRotation(
        ControlRotation
    );
}

void AKashmirCharacter::ApplyMovementConfig()
{
    if (MovementConfig == nullptr)
    {
        return;
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement == nullptr)
    {
        return;
    }

    Movement->MaxWalkSpeed = MovementConfig->WalkSpeed;
    Movement->MaxAcceleration = MovementConfig->MaxAcceleration;
    Movement->BrakingDecelerationWalking =
        MovementConfig->BrakingDecelerationWalking;
    Movement->GroundFriction = MovementConfig->GroundFriction;
    Movement->RotationRate =
        FRotator(0.0f, MovementConfig->RotationRateYaw, 0.0f);
}

float AKashmirCharacter::GetConfiguredWalkSpeed() const
{
    return MovementConfig
        ? MovementConfig->WalkSpeed
        : 450.0f;
}

float AKashmirCharacter::GetConfiguredSprintSpeed() const
{
    return MovementConfig
        ? MovementConfig->SprintSpeed
        : 650.0f;
}

void AKashmirCharacter::Move(const FInputActionValue& Value)
{
    if (Controller == nullptr)
    {
        return;
    }

    APlayerController* PlayerController =
        Cast<APlayerController>(Controller);

    const bool bLeftMouseDown =
        PlayerController &&
        PlayerController->IsInputKeyDown(EKeys::LeftMouseButton);

    const bool bRightMouseDown =
        PlayerController &&
        PlayerController->IsInputKeyDown(EKeys::RightMouseButton);

    const FVector2D RawInput =
        Value.Get<FVector2D>();

    if (bMovementStoppedSinceLastInput)
    {
        // Só movimento frontal positivo inicia o retorno.
        if (RawInput.Y > 0.0f &&
            !bLeftMouseDown &&
            !bRightMouseDown)
        {
            bCameraRecentering = true;
        }

        bMovementStoppedSinceLastInput = false;
    }

    const float StrafeMultiplier =
        MovementConfig
            ? MovementConfig->StrafeMultiplier
            : 0.70f;

    const float BackpedalMultiplier =
        MovementConfig
            ? MovementConfig->BackpedalMultiplier
            : 0.50f;

    FVector2D AdjustedInput = RawInput;

    AdjustedInput.X *= StrafeMultiplier;

    if (AdjustedInput.Y < 0.0f)
    {
        AdjustedInput.Y *= BackpedalMultiplier;
    }

    const FVector2D Input =
        AdjustedInput.GetClampedToMaxSize(1.0f);

    const FRotator ActorYaw(
        0.0f,
        GetActorRotation().Yaw,
        0.0f
    );

    AddMovementInput(
        FRotationMatrix(ActorYaw).GetUnitAxis(EAxis::X),
        Input.Y
    );

    AddMovementInput(
        FRotationMatrix(ActorYaw).GetUnitAxis(EAxis::Y),
        Input.X
    );
}
void AKashmirCharacter::Look(const FInputActionValue& Value)
{
    APlayerController* PlayerController =
        Cast<APlayerController>(Controller);

    if (PlayerController == nullptr)
    {
        return;
    }

    const bool bLeftMouseDown =
        PlayerController->IsInputKeyDown(EKeys::LeftMouseButton);

    const bool bRightMouseDown =
        PlayerController->IsInputKeyDown(EKeys::RightMouseButton);

    // Mouse sozinho não controla a câmera.
    if (!bLeftMouseDown && !bRightMouseDown)
    {
        return;
    }

    const FVector2D Input = Value.Get<FVector2D>();

    // LMB e RMB podem movimentar a câmera.
    AddControllerYawInput(Input.X);
    AddControllerPitchInput(Input.Y);

    // Somente RMB também altera o facing do personagem.
    if (bRightMouseDown)
    {
        const FRotator ControlRotation =
            PlayerController->GetControlRotation();

        SetActorRotation(
            FRotator(
                0.0f,
                ControlRotation.Yaw,
                0.0f
            )
        );
    }
}
void AKashmirCharacter::RequestDodge() { bDodgeRequested = true; UE_LOG(LogTemp, Display, TEXT("Kashmir: Dodge input received.")); }
void AKashmirCharacter::ToggleLockOn()
{
    if (IsValid(CurrentLockOnTarget))
    {
        ClearLockOnTarget();
        return;
    }

    UKashmirLockOnTargetComponent* BestTarget = FindBestLockOnTarget();

    if (BestTarget == nullptr)
    {
        bLockOnRequested = false;

        UE_LOG(
            LogTemp,
            Display,
            TEXT("Kashmir: no valid lock-on target found.")
        );

        return;
    }

    CurrentLockOnTarget = BestTarget->GetOwner();
    bLockOnRequested = true;

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Kashmir: locked on target '%s'."),
        *GetNameSafe(CurrentLockOnTarget)
    );
}

UKashmirLockOnTargetComponent*
AKashmirCharacter::FindBestLockOnTarget() const
{
    const UWorld* World = GetWorld();

    if (World == nullptr)
    {
        return nullptr;
    }

    const FVector CharacterLocation = GetActorLocation();

    FVector CameraLocation = CharacterLocation;
    FVector CameraForward = GetActorForwardVector();

    if (FollowCamera != nullptr)
    {
        CameraLocation = FollowCamera->GetComponentLocation();
        CameraForward = FollowCamera->GetForwardVector();
    }

    UKashmirLockOnTargetComponent* BestTarget = nullptr;
    float BestScore = -FLT_MAX;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* CandidateActor = *It;

        if (CandidateActor == nullptr || CandidateActor == this)
        {
            continue;
        }

        UKashmirLockOnTargetComponent* TargetComponent =
            CandidateActor->FindComponentByClass<
                UKashmirLockOnTargetComponent>();

        if (TargetComponent == nullptr ||
            !TargetComponent->CanBeLockedOn())
        {
            continue;
        }

        const FVector TargetLocation =
            TargetComponent->GetLockOnWorldLocation();

        const FVector ToTarget =
            TargetLocation - CameraLocation;

        const float Distance = ToTarget.Size();

        if (Distance <= KINDA_SMALL_NUMBER ||
            Distance > LockOnMaxDistance)
        {
            continue;
        }

        const FVector DirectionToTarget =
            ToTarget / Distance;

        const float CameraDot =
            FVector::DotProduct(
                CameraForward,
                DirectionToTarget
            );

        if (CameraDot < LockOnMinCameraDot)
        {
            continue;
        }

        /*
         * Camera alignment has the greatest influence.
         * Distance breaks ties between similarly aligned targets.
         * TargetPriority lets gameplay data influence selection.
         */
        const float NormalizedDistance =
            FMath::Clamp(
                Distance / LockOnMaxDistance,
                0.0f,
                1.0f
            );

        const float Score =
            (CameraDot * 2.0f)
            - NormalizedDistance
            + TargetComponent->TargetPriority;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = TargetComponent;
        }
    }

    return BestTarget;
}

void AKashmirCharacter::ClearLockOnTarget()
{
    if (IsValid(CurrentLockOnTarget))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Kashmir: unlocked target '%s'."),
            *GetNameSafe(CurrentLockOnTarget)
        );
    }

    CurrentLockOnTarget = nullptr;
    bLockOnRequested = false;
}

void AKashmirCharacter::BeginMouseTurnCharacter()
{
    bMouseTurnCharacter = true;
}

void AKashmirCharacter::EndMouseTurnCharacter()
{
    bMouseTurnCharacter = false;
}

void AKashmirCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bCameraRecentering || Controller == nullptr)
    {
        return;
    }

    APlayerController* PlayerController =
        Cast<APlayerController>(Controller);

    if (PlayerController == nullptr)
    {
        bCameraRecentering = false;
        return;
    }

    // Qualquer controle manual do mouse cancela o retorno.
    const bool bLeftMouseDown =
        PlayerController->IsInputKeyDown(EKeys::LeftMouseButton);

    const bool bRightMouseDown =
        PlayerController->IsInputKeyDown(EKeys::RightMouseButton);

    if (bLeftMouseDown || bRightMouseDown)
    {
        bCameraRecentering = false;
        return;
    }

    FRotator CurrentRotation =
        Controller->GetControlRotation();

    const float TargetYaw =
        GetActorRotation().Yaw;

    const float NewYaw =
        FMath::FInterpTo(
            CurrentRotation.Yaw,
            TargetYaw,
            DeltaSeconds,
            CameraRecenterSpeed
        );

    CurrentRotation.Yaw = NewYaw;

    Controller->SetControlRotation(CurrentRotation);

    const float RemainingDifference =
        FMath::Abs(
            FMath::FindDeltaAngleDegrees(
                NewYaw,
                TargetYaw
            )
        );

    if (RemainingDifference < 0.5f)
    {
        CurrentRotation.Yaw = TargetYaw;
        Controller->SetControlRotation(CurrentRotation);

        bCameraRecentering = false;
    }
}