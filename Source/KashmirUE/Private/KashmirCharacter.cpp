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
    if (LockOnBreakDistance < LockOnAcquireDistance)
    {
        LockOnBreakDistance =
            LockOnAcquireDistance;
    }
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

        Input->BindAction(
            TurnCharacterAction,
            ETriggerEvent::Completed,
            this,
            &AKashmirCharacter::HandleTurnCompleted
        );

        Input->BindAction(
            TurnCharacterAction,
            ETriggerEvent::Canceled,
            this,
            &AKashmirCharacter::HandleTurnCompleted
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

void AKashmirCharacter::HandleTurnCompleted(
    const FInputActionValue& Value)
{
    bManualTurnActive = false;
}

void AKashmirCharacter::HandleMoveCompleted(
    const FInputActionValue& Value)
{
    LastMoveInput2D = FVector2D::ZeroVector;
    bMovementStoppedSinceLastInput = true;
}

void AKashmirCharacter::TurnCharacter(
    const FInputActionValue& Value)
{
    if (Controller == nullptr)
    {
        return;
    }

    if (bIsDodging)
    {
        return;
    }

    const float Input = Value.Get<float>();

    if (FMath::IsNearlyZero(Input))
    {
        bManualTurnActive = false;
        return;
    }

//
// Manual turn explicitly leaves Lock-On.
//
    if (IsValid(CurrentLockOnTarget))
    {
        ClearLockOnTarget();

        bCameraRecentering = false;
    }

    bManualTurnActive = true;

    const float RotationSpeed =
        MovementConfig
            ? MovementConfig->RotationRateYaw
            : 120.0f;

    const float DeltaYaw =
        Input
        * RotationSpeed
        * GetWorld()->GetDeltaSeconds();

    //
    // Manual body rotation always has priority.
    //
    AddActorLocalRotation(
        FRotator(0.0f, DeltaYaw, 0.0f)
    );

    //
    // EXPLORATION:
    // A/D also rotate the camera by the same delta.
    //
    // LOCK-ON:
    // the camera remains controlled by the target tracker.
    //
    if (!IsValid(CurrentLockOnTarget))
    {
        FRotator ControlRotation =
            Controller->GetControlRotation();

        ControlRotation.Yaw += DeltaYaw;

        Controller->SetControlRotation(
            ControlRotation
        );
    }
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

    if (bIsDodging)
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
        LastMoveInput2D = RawInput;

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


//
// LOCK-ON MOVEMENT
//

if (IsValid(CurrentLockOnTarget))
{
    UKashmirLockOnTargetComponent* TargetComponent =
        CurrentLockOnTarget->FindComponentByClass<
            UKashmirLockOnTargetComponent>();

    if (TargetComponent != nullptr &&
        TargetComponent->CanBeLockedOn())
    {
        FVector ToTarget =
            TargetComponent->GetLockOnWorldLocation()
            - GetActorLocation();

        ToTarget.Z = 0.0f;

        const float DistanceToTarget =
            ToTarget.Size();

        if (DistanceToTarget > KINDA_SMALL_NUMBER)
        {
            const FVector ForwardToTarget =
                ToTarget / DistanceToTarget;

            const FVector RightAroundTarget =
                FVector::CrossProduct(
                    FVector::UpVector,
                    ForwardToTarget
                ).GetSafeNormal();

            //
            // W / S
            // aproxima ou afasta do alvo.
            //
            if (!FMath::IsNearlyZero(Input.Y))
            {
                AddMovementInput(
                    ForwardToTarget,
                    Input.Y
                );

                //
                // O jogador alterou voluntariamente a distância.
                // Essa nova distância passa a ser o novo raio orbital.
                //
                LockOnOrbitRadius =
                    DistanceToTarget;
            }

            //
            // Q / E
            // movimento tangencial + correção radial.
            //
            if (!FMath::IsNearlyZero(Input.X))
            {
                AddMovementInput(
                    RightAroundTarget,
                    Input.X
                );

                const float RadiusError =
                    DistanceToTarget
                    - LockOnOrbitRadius;

                const float OrbitCorrection =
                    FMath::Clamp(
                        RadiusError /
                            LockOnOrbitCorrectionRange,
                        -LockOnOrbitMaxCorrection,
                        LockOnOrbitMaxCorrection
                    );

                AddMovementInput(
                    ForwardToTarget,
                    OrbitCorrection
                );
            }

            return;
        }
    }
}

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
    if (IsValid(CurrentLockOnTarget))
    {
        return;
    }
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
void AKashmirCharacter::RequestDodge()
{
    if (bIsDodging)
    {
        return;
    }

    const FVector DodgeDirection =
        CalculateDodgeDirection();

    if (DodgeDirection.IsNearlyZero())
    {
        return;
    }

    bDodgeRequested = true;
    bIsDodging = true;

    ActiveDodgeDirection =
        DodgeDirection.GetSafeNormal();

    DodgeTimeRemaining =
        MovementConfig
            ? MovementConfig->DodgeDuration
            : 0.22f;

    bCameraRecentering = false;

    UE_LOG(
        LogTemp,
        Display,
        TEXT(
            "Kashmir: Dodge started. Direction=(%.2f, %.2f, %.2f)"
        ),
        ActiveDodgeDirection.X,
        ActiveDodgeDirection.Y,
        ActiveDodgeDirection.Z
    );
}

void AKashmirCharacter::UpdateDodge(
    float DeltaSeconds)
{
    if (!bIsDodging)
    {
        return;
    }

    UCharacterMovementComponent* Movement =
        GetCharacterMovement();

    if (Movement == nullptr)
    {
        bIsDodging = false;
        bDodgeRequested = false;
        return;
    }

    const float DodgeSpeed =
        MovementConfig
            ? MovementConfig->DodgeSpeed
            : 950.0f;

    //
    // Mantém Z atual para não destruir gravidade/quedas.
    //
    FVector NewVelocity =
        ActiveDodgeDirection * DodgeSpeed;

    NewVelocity.Z =
        Movement->Velocity.Z;

    Movement->Velocity =
        NewVelocity;

    DodgeTimeRemaining -=
        DeltaSeconds;

    if (DodgeTimeRemaining <= 0.0f)
    {
        bIsDodging = false;
        bDodgeRequested = false;

        DodgeTimeRemaining = 0.0f;
        ActiveDodgeDirection =
            FVector::ZeroVector;

        //
        // Remove apenas a velocidade horizontal criada pelo dodge.
        //
        Movement->Velocity.X = 0.0f;
        Movement->Velocity.Y = 0.0f;

        UE_LOG(
            LogTemp,
            Display,
            TEXT("Kashmir: Dodge completed.")
        );
    }
}

void AKashmirCharacter::ToggleLockOn()
{
    UKashmirLockOnTargetComponent* NextTarget =
        FindNextLockOnTarget();

    if (NextTarget == nullptr)
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Kashmir: no valid Lock-On target found.")
        );

        return;
    }

    AActor* PreviousTarget =
        CurrentLockOnTarget;

    CurrentLockOnTarget =
        NextTarget->GetOwner();

    bLockOnRequested = true;
    bCameraRecentering = false;
    LockOnLostSightTime = 0.0f;

    const FVector TargetLocation =
        NextTarget->GetLockOnWorldLocation();

    LockOnOrbitRadius =
        FVector::Dist2D(
            GetActorLocation(),
            TargetLocation
        );

    if (IsValid(PreviousTarget))
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Kashmir: switched Lock-On from '%s' to '%s'."),
            *GetNameSafe(PreviousTarget),
            *GetNameSafe(CurrentLockOnTarget)
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Display,
            TEXT("Kashmir: locked on target '%s'."),
            *GetNameSafe(CurrentLockOnTarget)
        );
    }
}

TArray<UKashmirLockOnTargetComponent*>
AKashmirCharacter::FindValidLockOnTargets() const
{
    TArray<UKashmirLockOnTargetComponent*> ValidTargets;

    const UWorld* World = GetWorld();

    if (World == nullptr)
    {
        return ValidTargets;
    }

    const FVector CharacterLocation =
        GetActorLocation();

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* CandidateActor = *It;

        if (CandidateActor == nullptr ||
            CandidateActor == this)
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

        const float Distance =
            FVector::Dist2D(
                CharacterLocation,
                TargetLocation
            );

        if (Distance <= KINDA_SMALL_NUMBER ||
            Distance > LockOnAcquireDistance)
        {
            continue;
        }

        if (!HasLineOfSightToLockOnTarget(TargetComponent))
        {
            continue;
        }

        ValidTargets.Add(TargetComponent);
    }

    //
    // Ordem determinística:
    // mais próximo -> mais distante.
    //
    ValidTargets.Sort(
        [CharacterLocation](
            const UKashmirLockOnTargetComponent& A,
            const UKashmirLockOnTargetComponent& B)
        {
            const float DistanceA =
                FVector::DistSquared2D(
                    CharacterLocation,
                    A.GetLockOnWorldLocation()
                );

            const float DistanceB =
                FVector::DistSquared2D(
                    CharacterLocation,
                    B.GetLockOnWorldLocation()
                );

            return DistanceA < DistanceB;
        }
    );

    return ValidTargets;
}

UKashmirLockOnTargetComponent*
AKashmirCharacter::FindNextLockOnTarget() const
{
    const TArray<UKashmirLockOnTargetComponent*> Targets =
        FindValidLockOnTargets();

    if (Targets.IsEmpty())
    {
        return nullptr;
    }

    // Sem alvo atual: começa pelo mais próximo.
    if (!IsValid(CurrentLockOnTarget))
    {
        return Targets[0];
    }

    for (int32 Index = 0; Index < Targets.Num(); ++Index)
    {
        UKashmirLockOnTargetComponent* TargetComponent =
            Targets[Index];

        if (TargetComponent != nullptr &&
            TargetComponent->GetOwner() == CurrentLockOnTarget)
        {
            const int32 NextIndex =
                (Index + 1) % Targets.Num();

            return Targets[NextIndex];
        }
    }

    // O alvo atual deixou de fazer parte da lista.
    return Targets[0];
}

bool AKashmirCharacter::HasLineOfSightToLockOnTarget(
    const UKashmirLockOnTargetComponent* TargetComponent) const
{
    if (TargetComponent == nullptr ||
        FollowCamera == nullptr ||
        GetWorld() == nullptr)
    {
        return false;
    }

    const AActor* TargetActor =
        TargetComponent->GetOwner();

    if (!IsValid(TargetActor))
    {
        return false;
    }

    const FVector TraceStart =
        FollowCamera->GetComponentLocation();

    const FVector TraceEnd =
        TargetComponent->GetLockOnWorldLocation();

    FCollisionQueryParams QueryParams;

    QueryParams.AddIgnoredActor(this);

    FHitResult HitResult;

    const bool bHit =
        GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

    // Nenhum bloqueio até o ponto do alvo.
    if (!bHit)
    {
        return true;
    }

    // O primeiro objeto atingido foi o próprio target.
    return HitResult.GetActor() == TargetActor;
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
    LockOnOrbitRadius = 0.0f;
    LockOnLostSightTime = 0.0f;
}

void AKashmirCharacter::BeginMouseTurnCharacter()
{
    bMouseTurnCharacter = true;
}

void AKashmirCharacter::EndMouseTurnCharacter()
{
    bMouseTurnCharacter = false;
}

void AKashmirCharacter::UpdateLockOn(float DeltaSeconds)
{
    if (!IsValid(CurrentLockOnTarget) || Controller == nullptr)
    {
        ClearLockOnTarget();
        return;
    }

    UKashmirLockOnTargetComponent* TargetComponent =
        CurrentLockOnTarget->FindComponentByClass<
            UKashmirLockOnTargetComponent>();

    if (TargetComponent == nullptr ||
        !TargetComponent->CanBeLockedOn())
    {
        ClearLockOnTarget();
        return;
    }

    const FVector TargetLocation =
        TargetComponent->GetLockOnWorldLocation();

    const float DistanceToTarget =
        FVector::Dist2D(
            GetActorLocation(),
            TargetLocation
        );

    if (DistanceToTarget > LockOnBreakDistance)
    {
        ClearLockOnTarget();
        return;
    }


//
// LINE OF SIGHT
//

    if (HasLineOfSightToLockOnTarget(TargetComponent))
    {
        LockOnLostSightTime = 0.0f;
    }
    else
    {
        LockOnLostSightTime += DeltaSeconds;

        if (LockOnLostSightTime >=
            LockOnLineOfSightGraceTime)
        {
            UE_LOG(
                LogTemp,
                Display,
                TEXT("Kashmir: Lock-On lost due to line of sight.")
            );

            ClearLockOnTarget();
            return;
        }
    }

//
// BODY
//

if (!bManualTurnActive)
{
    const FVector CharacterLocation =
        GetActorLocation();

    FVector BodyDirection =
        TargetLocation - CharacterLocation;

    BodyDirection.Z = 0.0f;

    if (!BodyDirection.IsNearlyZero())
    {
        const FRotator DesiredBodyRotation =
            BodyDirection.Rotation();

        const FRotator CurrentBodyRotation =
            GetActorRotation();

        const FRotator NewBodyRotation =
            FMath::RInterpTo(
                CurrentBodyRotation,
                FRotator(
                    0.0f,
                    DesiredBodyRotation.Yaw,
                    0.0f
                ),
                DeltaSeconds,
                LockOnBodyRotationSpeed
            );

        SetActorRotation(
            FRotator(
                0.0f,
                NewBodyRotation.Yaw,
                0.0f
            )
        );
    }
}

    
    //
    // CAMERA
    //

    if (FollowCamera != nullptr)
    {
        const FVector CameraLocation =
            FollowCamera->GetComponentLocation();

        const FVector CameraToTarget =
            TargetLocation - CameraLocation;

        if (!CameraToTarget.IsNearlyZero())
        {
            const FRotator DesiredCameraRotation =
                CameraToTarget.Rotation();

            const FRotator CurrentControlRotation =
                Controller->GetControlRotation();

            const FRotator NewControlRotation =
                FMath::RInterpTo(
                    CurrentControlRotation,
                    DesiredCameraRotation,
                    DeltaSeconds,
                    LockOnCameraRotationSpeed
                );

            Controller->SetControlRotation(
                NewControlRotation
            );
        }
    }
}

void AKashmirCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    //
    // 1. Movimento autoritativo de curta duração.
    //
    if (bIsDodging)
    {
        UpdateDodge(DeltaSeconds);
    }

    //
    // 2. Estado de Lock-On.
    //
    if (IsValid(CurrentLockOnTarget))
    {
        UpdateLockOn(DeltaSeconds);
        return;
    }

    //
    // 3. Assistência normal de câmera.
    //
    if (!bCameraRecentering ||
        Controller == nullptr)
    {
        return;
    }

    // restante atual do recenter...

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

    const float ShortestYawDelta =
        FMath::FindDeltaAngleDegrees(
            CurrentRotation.Yaw,
            TargetYaw
        );

    const float ShortestTargetYaw =
        CurrentRotation.Yaw + ShortestYawDelta;

    const float NewYaw =
        FMath::FInterpTo(
            CurrentRotation.Yaw,
            ShortestTargetYaw,
            DeltaSeconds,
            CameraRecenterSpeed
        );

    CurrentRotation.Yaw =
        FRotator::NormalizeAxis(NewYaw);

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

FVector AKashmirCharacter::CalculateDodgeDirection() const
{
    FVector2D DodgeInput = LastMoveInput2D;

    const APlayerController* PlayerController =
        Cast<APlayerController>(Controller);

    //
    // LMB + RMB também representa movimento frontal.
    //
    if (PlayerController != nullptr)
    {
        const bool bLeftMouseDown =
            PlayerController->IsInputKeyDown(
                EKeys::LeftMouseButton
            );

        const bool bRightMouseDown =
            PlayerController->IsInputKeyDown(
                EKeys::RightMouseButton
            );

        if (bLeftMouseDown &&
            bRightMouseDown &&
            DodgeInput.IsNearlyZero())
        {
            DodgeInput.Y = 1.0f;
        }
    }

    //
    // LOCK-ON
    //
    if (IsValid(CurrentLockOnTarget))
    {
        UKashmirLockOnTargetComponent* TargetComponent =
            CurrentLockOnTarget->FindComponentByClass<
                UKashmirLockOnTargetComponent>();

        if (TargetComponent != nullptr)
        {
            FVector ToTarget =
                TargetComponent->GetLockOnWorldLocation()
                - GetActorLocation();

            ToTarget.Z = 0.0f;

            if (!ToTarget.IsNearlyZero())
            {
                const FVector ForwardToTarget =
                    ToTarget.GetSafeNormal();

                const FVector RightAroundTarget =
                    FVector::CrossProduct(
                        FVector::UpVector,
                        ForwardToTarget
                    ).GetSafeNormal();

                //
                // Sem input durante Lock-On:
                // dodge para trás.
                //
                if (DodgeInput.IsNearlyZero())
                {
                    return -ForwardToTarget;
                }

                DodgeInput =
                    DodgeInput.GetClampedToMaxSize(1.0f);

                const FVector Direction =
                    ForwardToTarget * DodgeInput.Y
                    + RightAroundTarget * DodgeInput.X;

                return Direction.GetSafeNormal();
            }
        }
    }

    //
    // EXPLORAÇÃO
    //
    const FRotator ActorYaw(
        0.0f,
        GetActorRotation().Yaw,
        0.0f
    );

    const FVector Forward =
        FRotationMatrix(ActorYaw).GetUnitAxis(EAxis::X);

    const FVector Right =
        FRotationMatrix(ActorYaw).GetUnitAxis(EAxis::Y);

    //
    // Sem input:
    // dodge para frente.
    //
    if (DodgeInput.IsNearlyZero())
    {
        return Forward;
    }

    DodgeInput =
        DodgeInput.GetClampedToMaxSize(1.0f);

    return (
        Forward * DodgeInput.Y
        + Right * DodgeInput.X
    ).GetSafeNormal();
}