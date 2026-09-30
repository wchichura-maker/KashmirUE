#include "Combat/KashmirMovementDeliveryComponent.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UKashmirMovementDeliveryComponent::UKashmirMovementDeliveryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


bool UKashmirMovementDeliveryComponent::CanStartDelivery(
    const FKashmirTechniqueMovementSpec& Spec,
    FString& OutReason) const
{
    if (!Spec.IsValid(OutReason))
    {
        return false;
    }
    if (Spec.Delivery == EKashmirMovementDelivery::None)
    {
        return true;
    }
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character == nullptr || Character->GetCharacterMovement() == nullptr ||
        Character->GetCharacterMovement()->UpdatedComponent == nullptr)
    {
        OutReason = TEXT("controlled translation requires an owning CharacterMovement component");
        return false;
    }
    return true;
}


bool UKashmirMovementDeliveryComponent::StartDelivery(
    const FKashmirTechniqueMovementSpec& Spec,
    const FName ActionId,
    FString& OutReason)
{
    if (!CanStartDelivery(Spec, OutReason))
    {
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Invalid);
        return false;
    }

    ResetDeliveryState();
    ActiveSpec = Spec;
    ActiveActionId = ActionId;
    RequestedDistance = Spec.Distance;
    if (Spec.Delivery == EKashmirMovementDelivery::None)
    {
        return true;
    }

    const ACharacter* Character = CastChecked<ACharacter>(GetOwner());
    MovementDirection = Character->GetActorForwardVector();
    MovementDirection.Z = 0.0f;
    MovementDirection = MovementDirection.GetSafeNormal();
    if (ActionId.IsNone() || MovementDirection.IsNearlyZero())
    {
        OutReason = TEXT("controlled translation requires an action id and horizontal character forward");
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Invalid);
        return false;
    }

    RequestedVelocity = MovementDirection * (Spec.Distance / Spec.Duration);
    bDeliveryActive = true;
    return true;
}


bool UKashmirMovementDeliveryComponent::AdvanceDelivery(
    const float DeltaSeconds,
    const FKashmirActionRuntimeState& ActionState,
    FString& OutReason)
{
    OutReason.Reset();
    LastRequestedDelta = FVector::ZeroVector;
    if (!bDeliveryActive)
    {
        return true;
    }
    if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds < 0.0f)
    {
        OutReason = TEXT("movement delivery delta time must be finite and non-negative");
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Invalid);
        return false;
    }
    if (!ActionState.bActive || ActionState.ActionId != ActiveActionId)
    {
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::ActionEnded);
        return true;
    }
    if (DeltaSeconds <= UE_SMALL_NUMBER)
    {
        return true;
    }

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    UCharacterMovementComponent* Movement = Character != nullptr
        ? Character->GetCharacterMovement() : nullptr;
    if (Movement == nullptr || Movement->UpdatedComponent == nullptr)
    {
        OutReason = TEXT("movement delivery lost its CharacterMovement authority");
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Invalid);
        return false;
    }

    const float RemainingTime = FMath::Max(0.0f, ActiveSpec.Duration - ElapsedSeconds);
    const float StepSeconds = FMath::Min(DeltaSeconds, RemainingTime);
    const float RemainingDistance = FMath::Max(0.0f, RequestedDistance - ActualDistance);
    const float RequestedStepDistance = FMath::Min(
        RequestedVelocity.Size() * StepSeconds,
        RemainingDistance);
    LastRequestedDelta = MovementDirection * RequestedStepDistance;

    const FVector Before = Movement->UpdatedComponent->GetComponentLocation();
    FHitResult Hit;
    Movement->SafeMoveUpdatedComponent(
        LastRequestedDelta,
        Movement->UpdatedComponent->GetComponentQuat(),
        true,
        Hit);
    const FVector After = Movement->UpdatedComponent->GetComponentLocation();
    const float ActualStepDistance = FVector::Dist2D(Before, After);
    ActualDistance += ActualStepDistance;
    ElapsedSeconds += StepSeconds;

    const bool bStepBlocked = Hit.IsValidBlockingHit() ||
        ActualStepDistance + 0.1f < RequestedStepDistance;
    if (bStepBlocked)
    {
        bBlocked = true;
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Blocked);
        return true;
    }
    if (ElapsedSeconds + UE_SMALL_NUMBER >= ActiveSpec.Duration ||
        ActualDistance + 0.1f >= RequestedDistance)
    {
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Completed);
    }
    return true;
}


void UKashmirMovementDeliveryComponent::CancelDelivery()
{
    if (bDeliveryActive)
    {
        FinishDelivery(EKashmirMovementDeliveryCompletionReason::Cancelled);
    }
}


void UKashmirMovementDeliveryComponent::FinishDelivery(
    const EKashmirMovementDeliveryCompletionReason Reason)
{
    bDeliveryActive = false;
    RequestedVelocity = FVector::ZeroVector;
    LastRequestedDelta = FVector::ZeroVector;
    CompletionReason = Reason;
}


void UKashmirMovementDeliveryComponent::ResetDeliveryState()
{
    ActiveSpec = {};
    ActiveActionId = NAME_None;
    MovementDirection = FVector::ZeroVector;
    RequestedVelocity = FVector::ZeroVector;
    LastRequestedDelta = FVector::ZeroVector;
    RequestedDistance = 0.0f;
    ActualDistance = 0.0f;
    ElapsedSeconds = 0.0f;
    bDeliveryActive = false;
    bBlocked = false;
    CompletionReason = EKashmirMovementDeliveryCompletionReason::None;
}


#if WITH_EDITOR
AActor* UKashmirMovementDeliveryComponent::SpawnTransientDebugBlocker(
    const FVector WorldLocation,
    const FVector BoxExtent,
    FString& OutReason)
{
    OutReason.Reset();
    UWorld* World = GetWorld();
    if (World == nullptr || WorldLocation.ContainsNaN() ||
        BoxExtent.ContainsNaN() ||
        BoxExtent.X <= 0.0f || BoxExtent.Y <= 0.0f || BoxExtent.Z <= 0.0f)
    {
        OutReason = TEXT("debug blocker requires a valid world, location, and positive extent");
        return nullptr;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.ObjectFlags |= RF_Transient;
    AActor* Blocker = World->SpawnActor<AActor>(
        AActor::StaticClass(), FTransform::Identity, SpawnParameters);
    if (Blocker == nullptr)
    {
        OutReason = TEXT("failed to spawn transient movement delivery blocker");
        return nullptr;
    }

    UBoxComponent* Box = NewObject<UBoxComponent>(
        Blocker, TEXT("MovementDeliveryDebugBlocker"), RF_Transient);
    Blocker->AddInstanceComponent(Box);
    Box->SetBoxExtent(BoxExtent);
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->RegisterComponentWithWorld(World);
    Blocker->SetRootComponent(Box);
    Box->SetWorldLocation(
        WorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
    Box->UpdateComponentToWorld();
    return Blocker;
}
#endif
