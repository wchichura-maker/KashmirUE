#include "KashmirAnimInstance.h"

#include "KashmirCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Combat/KashmirSwordPresentationComponent.h"

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

    SwordPose = {};
    SwordRigCurves = {};

    if (Character == nullptr)
    {
        Character =
            Cast<AKashmirCharacter>(
                TryGetPawnOwner()
            );
    }

    if (Character == nullptr)
    {
        PublishSwordRigCurves();
        return;
    }

    const UCharacterMovementComponent* Movement =
        Character->GetCharacterMovement();

    if (Movement == nullptr)
    {
        PublishSwordRigCurves();
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

    UpdateSwordPresentation();
}


void UKashmirAnimInstance::UpdateSwordPresentation()
{
    const UKashmirSwordPresentationComponent* SwordPresentation =
        Character != nullptr
            ? Character->GetSwordPresentationComponent()
            : nullptr;
    SwordPose = SwordPresentation != nullptr
        ? SwordPresentation->GetCurrentPose()
        : FKashmirSwordPoseResult{};

    FKashmirSwordRigAdapter Adapter;
    FString Reason;
    if (!Adapter.Resolve(SwordPose, SwordRigCurves, Reason))
    {
        SwordRigCurves = {};
    }

    PublishSwordRigCurves();
}


void UKashmirAnimInstance::PublishSwordRigCurves()
{
    AddCurveValue(KashmirSwordRigCurves::LeadHandOffsetX, SwordRigCurves.LeadHandOffset.X);
    AddCurveValue(KashmirSwordRigCurves::LeadHandOffsetY, SwordRigCurves.LeadHandOffset.Y);
    AddCurveValue(KashmirSwordRigCurves::LeadHandOffsetZ, SwordRigCurves.LeadHandOffset.Z);
    AddCurveValue(KashmirSwordRigCurves::SupportHandOffsetX, SwordRigCurves.SupportHandOffset.X);
    AddCurveValue(KashmirSwordRigCurves::SupportHandOffsetY, SwordRigCurves.SupportHandOffset.Y);
    AddCurveValue(KashmirSwordRigCurves::SupportHandOffsetZ, SwordRigCurves.SupportHandOffset.Z);
    AddCurveValue(KashmirSwordRigCurves::AimPitch, SwordRigCurves.AimRotation.Pitch);
    AddCurveValue(KashmirSwordRigCurves::AimYaw, SwordRigCurves.AimRotation.Yaw);
    AddCurveValue(KashmirSwordRigCurves::AimRoll, SwordRigCurves.AimRotation.Roll);
    AddCurveValue(KashmirSwordRigCurves::BodyLean, SwordRigCurves.BodyLeanDegrees);
    AddCurveValue(KashmirSwordRigCurves::SwordPoseAlpha, SwordRigCurves.SwordPoseAlpha);
    AddCurveValue(KashmirSwordRigCurves::LeftFootLockAlpha, SwordRigCurves.LeftFootLockAlpha);
    AddCurveValue(KashmirSwordRigCurves::RightFootLockAlpha, SwordRigCurves.RightFootLockAlpha);
}
