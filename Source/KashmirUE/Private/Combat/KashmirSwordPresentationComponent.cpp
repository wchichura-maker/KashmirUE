#include "Combat/KashmirSwordPresentationComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "KashmirAnimInstance.h"


namespace
{
    bool ValidatePresentationPlan(
        const FKashmirSwordActionPlan& Plan,
        FString& OutReason)
    {
        OutReason.Reset();

        if (!Plan.bResolved)
        {
            OutReason = TEXT("sword presentation requires a resolved action plan");
            return false;
        }

        if (!Plan.RuntimeDefinition.IsValid(OutReason))
        {
            OutReason = FString::Printf(TEXT("invalid sword presentation runtime definition: %s"), *OutReason);
            return false;
        }

        if (Plan.Montage.IsNull())
        {
            OutReason = TEXT("sword presentation requires a montage");
            return false;
        }

        if (!FMath::IsFinite(Plan.PlayRate) || Plan.PlayRate <= 0.0f)
        {
            OutReason = TEXT("sword presentation play rate must be finite and positive");
            return false;
        }

        return true;
    }
}


bool FKashmirSwordPresentationSyncResolver::Resolve(
    const FKashmirSwordActionPlan& Plan,
    const FKashmirActionRuntimeState& RuntimeState,
    const FKashmirSwordPresentationState& PresentationState,
    const bool bExpectedMontageActive,
    FKashmirSwordPresentationSyncResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!ValidatePresentationPlan(Plan, OutReason))
    {
        return false;
    }

    if (!FMath::IsFinite(RuntimeState.Elapsed) || RuntimeState.Elapsed < 0.0f)
    {
        OutReason = TEXT("sword presentation runtime elapsed time must be finite and non-negative");
        return false;
    }

    const bool bMatchingActiveAction =
        RuntimeState.bActive &&
        RuntimeState.ActionId == Plan.RuntimeDefinition.ActionId;

    if (!bMatchingActiveAction)
    {
        OutResult.Command =
            PresentationState.bActive
                ? EKashmirSwordPresentationCommand::Stop
                : EKashmirSwordPresentationCommand::None;
        return true;
    }

    OutResult.Command =
        !PresentationState.bActive ||
        PresentationState.ActionId != RuntimeState.ActionId ||
        !bExpectedMontageActive
            ? EKashmirSwordPresentationCommand::Play
            : EKashmirSwordPresentationCommand::Synchronize;
    OutResult.MontagePositionSeconds = RuntimeState.Elapsed * Plan.PlayRate;
    OutResult.PlayRate = Plan.PlayRate;
    return true;
}


UKashmirSwordPresentationComponent::UKashmirSwordPresentationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


void UKashmirSwordPresentationComponent::SetSkeletalMesh(
    USkeletalMeshComponent* InSkeletalMesh)
{
    if (SkeletalMesh != InSkeletalMesh)
    {
        StopPresentation(0.0f);
        SkeletalMesh = InSkeletalMesh;
    }
}


bool UKashmirSwordPresentationComponent::ApplyRuntimeState(
    const FKashmirSwordActionPlan& Plan,
    const FKashmirActionRuntimeState& RuntimeState,
    FString& OutReason)
{
    UAnimInstance* AnimInstance = SkeletalMesh != nullptr
        ? SkeletalMesh->GetAnimInstance()
        : nullptr;
    UAnimMontage* ExpectedMontage = Plan.Montage.LoadSynchronous();
    const bool bExpectedMontageActive =
        AnimInstance != nullptr &&
        ExpectedMontage != nullptr &&
        ActiveMontage == ExpectedMontage &&
        AnimInstance->Montage_IsActive(ExpectedMontage);

    FKashmirSwordPresentationSyncResult Sync;
    FKashmirSwordPresentationSyncResolver Resolver;
    if (!Resolver.Resolve(
            Plan,
            RuntimeState,
            PresentationState,
            bExpectedMontageActive,
            Sync,
            OutReason))
    {
        return false;
    }

    FKashmirSwordPoseResult ResolvedPose;
    FKashmirSwordPoseResolver PoseResolver;
    if (!PoseResolver.Resolve(
            Plan.Gesture,
            Plan.RuntimeDefinition,
            Plan.PoseConfig,
            RuntimeState,
            ResolvedPose,
            OutReason))
    {
        return false;
    }
    CurrentPose = ResolvedPose;

    if (Sync.Command == EKashmirSwordPresentationCommand::None)
    {
        return true;
    }

    if (Sync.Command == EKashmirSwordPresentationCommand::Stop)
    {
        StopPresentation();
        return true;
    }

    if (SkeletalMesh == nullptr)
    {
        OutReason = TEXT("sword presentation requires a skeletal mesh");
        return false;
    }

    AnimInstance = SkeletalMesh->GetAnimInstance();
    if (AnimInstance == nullptr)
    {
        OutReason = TEXT("sword presentation requires an animation instance");
        return false;
    }

    if (UKashmirAnimInstance* KashmirAnimInstance =
            Cast<UKashmirAnimInstance>(AnimInstance))
    {
        KashmirAnimInstance->SetSwordMovementIntent(Plan.MovementIntent);
    }

    UAnimMontage* Montage = ExpectedMontage;
    if (Montage == nullptr)
    {
        OutReason = TEXT("sword presentation montage could not be loaded");
        return false;
    }

    const float MontagePosition =
        FMath::Clamp(Sync.MontagePositionSeconds, 0.0f, Montage->GetPlayLength());

    if (Sync.Command == EKashmirSwordPresentationCommand::Play)
    {
        if (AnimInstance->Montage_Play(
                Montage,
                Sync.PlayRate,
                EMontagePlayReturnType::MontageLength,
                MontagePosition,
                false) <= 0.0f)
        {
            OutReason = TEXT("sword presentation montage failed to play");
            return false;
        }

        if (!Plan.MontageSection.IsNone())
        {
            AnimInstance->Montage_JumpToSection(Plan.MontageSection, Montage);
        }

        ActiveMontage = Montage;
        PresentationState.bActive = true;
        PresentationState.ActionId = RuntimeState.ActionId;
    }
    else
    {
        AnimInstance->Montage_SetPosition(Montage, MontagePosition);
        AnimInstance->Montage_SetPlayRate(Montage, Sync.PlayRate);
    }

    OutReason.Reset();
    return true;
}


void UKashmirSwordPresentationComponent::StopPresentation(float BlendOutSeconds)
{
    if (SkeletalMesh != nullptr)
    {
        if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
        {
            if (ActiveMontage != nullptr)
            {
                AnimInstance->Montage_Stop(FMath::Max(0.0f, BlendOutSeconds), ActiveMontage);
            }
            if (UKashmirAnimInstance* KashmirAnimInstance =
                    Cast<UKashmirAnimInstance>(AnimInstance))
            {
                KashmirAnimInstance->SetSwordMovementIntent(
                    EKashmirMovementIntent::Stationary);
            }
        }
    }

    ActiveMontage = nullptr;
    PresentationState = {};
    CurrentPose = {};
}
