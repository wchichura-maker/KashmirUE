#include "KashmirCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AKashmirCharacter::AKashmirCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    bUseControllerRotationPitch = false; bUseControllerRotationYaw = false; bUseControllerRotationRoll = false;
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->bOrientRotationToMovement = true; Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); Movement->MaxWalkSpeed = 500.0f; Movement->BrakingDecelerationWalking = 1800.0f;
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent); CameraBoom->TargetArmLength = 360.0f; CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 70.0f); CameraBoom->bUsePawnControlRotation = true;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); FollowCamera->bUsePawnControlRotation = false;
}
void AKashmirCharacter::BeginPlay()
{
    Super::BeginPlay();
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
    if (MoveAction) Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKashmirCharacter::Move);
    if (LookAction) Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKashmirCharacter::Look);
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
void AKashmirCharacter::Move(const FInputActionValue& Value)
{
    if (Controller == nullptr) return;

    const FVector2D RawInput = Value.Get<FVector2D>();
    const FVector2D Input = RawInput.GetClampedToMaxSize(1.0f);

    const FRotator ActorYaw(0.0f, GetActorRotation().Yaw, 0.0f);

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
    const FVector2D Input = Value.Get<FVector2D>();

    AddControllerYawInput(Input.X);
    AddControllerPitchInput(Input.Y);

    if (bMouseTurnCharacter)
    {
        const FRotator ControlRotation = Controller->GetControlRotation();
        SetActorRotation(FRotator(0.0f, ControlRotation.Yaw, 0.0f));
    }
}
void AKashmirCharacter::RequestDodge() { bDodgeRequested = true; UE_LOG(LogTemp, Display, TEXT("Kashmir: Dodge input received.")); }
void AKashmirCharacter::ToggleLockOn() { bLockOnRequested = !bLockOnRequested; UE_LOG(LogTemp, Display, TEXT("Kashmir: Lock-on intent is %s."), bLockOnRequested ? TEXT("on") : TEXT("off")); }
void AKashmirCharacter::BeginMouseTurnCharacter()
{
    bMouseTurnCharacter = true;
}

void AKashmirCharacter::EndMouseTurnCharacter()
{
    bMouseTurnCharacter = false;
}