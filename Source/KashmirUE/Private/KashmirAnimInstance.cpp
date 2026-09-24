#include "KashmirAnimInstance.h"

#include "KashmirCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UKashmirAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Character =
        Cast<AKashmirCharacter>(
            TryGetPawnOwner()
        );
}

void UKashmirAnimInstance::NativeUpdateAnimation(
    float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (Character == nullptr)
    {
        Character =
            Cast<AKashmirCharacter>(
                TryGetPawnOwner()
            );
    }

    if (Character == nullptr)
    {
        return;
    }

    const UCharacterMovementComponent* Movement =
        Character->GetCharacterMovement();

    if (Movement == nullptr)
    {
        return;
    }

    const FVector Velocity =
        Movement->Velocity;

    const FVector HorizontalVelocity(
        Velocity.X,
        Velocity.Y,
        0.0f
    );

    GroundSpeed =
        HorizontalVelocity.Size();

    Direction =
        UKismetAnimationLibrary::CalculateDirection(
            HorizontalVelocity,
            Character->GetActorRotation()
        );

    bShouldMove =
        GroundSpeed > 3.0f;

    bIsFalling =
        Movement->IsFalling();

    bIsLockedOn =
        Character->IsLockOnActive();

    bIsDodging =
        Character->IsDodging();

    bIsWalking =
        Character->IsWalking();
}
