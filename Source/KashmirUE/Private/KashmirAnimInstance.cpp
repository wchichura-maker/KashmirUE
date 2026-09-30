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
    ResetSwordRigInputs();

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

    ResolveSwordRigInputsFromCurrentPose();
}


void UKashmirAnimInstance::ResolveSwordRigInputsFromCurrentPose()
{
#if !UE_BUILD_SHIPPING
    if (bForceNeutralSwordRigInputs)
    {
        ResetSwordRigInputs();
        return;
    }
#endif

    FKashmirSwordRigInputs Inputs;
    FKashmirSwordRigAdapter Adapter;
    FString Reason;
    if (!Adapter.Resolve(SwordPose, Inputs, Reason))
    {
        Inputs = {};
    }
    ApplySwordRigInputs(Inputs);
}


void UKashmirAnimInstance::SetForceNeutralSwordRigInputs(bool bForceNeutral)
{
#if !UE_BUILD_SHIPPING
    bForceNeutralSwordRigInputs = bForceNeutral;
#else
    bForceNeutralSwordRigInputs = false;
#endif
    ResolveSwordRigInputsFromCurrentPose();
}


bool UKashmirAnimInstance::IsForceNeutralSwordRigInputsEnabled() const
{
#if !UE_BUILD_SHIPPING
    return bForceNeutralSwordRigInputs;
#else
    return false;
#endif
}


void UKashmirAnimInstance::SetSwordMovementIntent(
    const EKashmirMovementIntent InMovementIntent)
{
    SwordMovementIntent = InMovementIntent;
    bUseFullBodySwordMontage =
        SwordMovementIntent == EKashmirMovementIntent::FullBody;
}


FKashmirSwordRigInputs UKashmirAnimInstance::GetSwordRigInputs() const
{
    FKashmirSwordRigInputs Inputs;
    Inputs.LeadHandOffset = FVector(SwordLeadHandOffsetX, SwordLeadHandOffsetY, SwordLeadHandOffsetZ);
    Inputs.SupportHandOffset = FVector(SwordSupportHandOffsetX, SwordSupportHandOffsetY, SwordSupportHandOffsetZ);
    Inputs.AimRotation = FRotator(SwordAimPitch, SwordAimYaw, SwordAimRoll);
    Inputs.BodyLeanDegrees = SwordBodyLean;
    Inputs.SwordPoseAlpha = SwordPoseAlpha;
    Inputs.LeftFootLockAlpha = SwordLeftFootLockAlpha;
    Inputs.RightFootLockAlpha = SwordRightFootLockAlpha;
    return Inputs;
}


void UKashmirAnimInstance::ApplySwordRigInputs(const FKashmirSwordRigInputs& Inputs)
{
    SwordLeadHandOffsetX = Inputs.LeadHandOffset.X;
    SwordLeadHandOffsetY = Inputs.LeadHandOffset.Y;
    SwordLeadHandOffsetZ = Inputs.LeadHandOffset.Z;
    SwordSupportHandOffsetX = Inputs.SupportHandOffset.X;
    SwordSupportHandOffsetY = Inputs.SupportHandOffset.Y;
    SwordSupportHandOffsetZ = Inputs.SupportHandOffset.Z;
    SwordAimPitch = Inputs.AimRotation.Pitch;
    SwordAimYaw = Inputs.AimRotation.Yaw;
    SwordAimRoll = Inputs.AimRotation.Roll;
    SwordBodyLean = Inputs.BodyLeanDegrees;
    SwordPoseAlpha = Inputs.SwordPoseAlpha;
    SwordLeftFootLockAlpha = Inputs.LeftFootLockAlpha;
    SwordRightFootLockAlpha = Inputs.RightFootLockAlpha;
}


void UKashmirAnimInstance::ResetSwordRigInputs()
{
    ApplySwordRigInputs({});
}
