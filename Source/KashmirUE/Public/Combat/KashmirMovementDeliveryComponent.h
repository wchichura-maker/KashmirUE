#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/KashmirCombatTechnique.h"
#include "Runtime/KashmirActionRuntime.h"

#include "KashmirMovementDeliveryComponent.generated.h"


UENUM(BlueprintType)
enum class EKashmirMovementDeliveryCompletionReason : uint8
{
    None,
    Completed,
    Cancelled,
    Transitioned,
    ActionEnded,
    Blocked,
    Invalid
};


/**
 * Executes authored Technique displacement through CharacterMovement.
 * The owning ActionRuntime drives this component explicitly; it has no phase
 * machine or tick of its own and never owns presentation or combat results.
 */
UCLASS(ClassGroup=(Kashmir), meta=(BlueprintSpawnableComponent))
class KASHMIRUE_API UKashmirMovementDeliveryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKashmirMovementDeliveryComponent();

    bool CanStartDelivery(
        const FKashmirTechniqueMovementSpec& Spec,
        FString& OutReason) const;

    bool StartDelivery(
        const FKashmirTechniqueMovementSpec& Spec,
        FName ActionId,
        FString& OutReason);

    bool AdvanceDelivery(
        float DeltaSeconds,
        const FKashmirActionRuntimeState& ActionState,
        FString& OutReason);

    void CancelDelivery();

    /** Ends the current delivery because its owning Technique transitioned. */
    void TransitionDelivery();

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    bool IsDeliveryActive() const { return bDeliveryActive; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    float GetRequestedDistance() const { return RequestedDistance; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    float GetActualDistance() const { return ActualDistance; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    float GetElapsedSeconds() const { return ElapsedSeconds; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    FVector GetRequestedVelocity() const { return RequestedVelocity; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    FVector GetLastRequestedDelta() const { return LastRequestedDelta; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    bool WasBlocked() const { return bBlocked; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    EKashmirMovementDeliveryCompletionReason GetCompletionReason() const
    { return CompletionReason; }

    UFUNCTION(BlueprintPure, Category="Combat|Movement Delivery|Debug", meta=(DevelopmentOnly))
    EKashmirMovementDeliveryCompletionReason GetLastCompletionReason() const
    { return LastCompletionReason; }

#if WITH_EDITOR
    /** Creates unsaved blocking geometry for PIE validation only. */
    UFUNCTION(BlueprintCallable, Category="Combat|Movement Delivery|Development",
        meta=(DevelopmentOnly))
    AActor* SpawnTransientDebugBlocker(
        FVector WorldLocation,
        FVector BoxExtent,
        FString& OutReason);
#endif

private:
    void FinishDelivery(EKashmirMovementDeliveryCompletionReason Reason);
    void ResetDeliveryState();

    UPROPERTY(Transient)
    FKashmirTechniqueMovementSpec ActiveSpec;

    UPROPERTY(Transient)
    FName ActiveActionId;

    UPROPERTY(Transient)
    FVector MovementDirection = FVector::ZeroVector;

    UPROPERTY(Transient)
    FVector RequestedVelocity = FVector::ZeroVector;

    UPROPERTY(Transient)
    FVector LastRequestedDelta = FVector::ZeroVector;

    UPROPERTY(Transient)
    float RequestedDistance = 0.0f;

    UPROPERTY(Transient)
    float ActualDistance = 0.0f;

    UPROPERTY(Transient)
    float ElapsedSeconds = 0.0f;

    UPROPERTY(Transient)
    bool bDeliveryActive = false;

    UPROPERTY(Transient)
    bool bBlocked = false;

    UPROPERTY(Transient)
    EKashmirMovementDeliveryCompletionReason CompletionReason =
        EKashmirMovementDeliveryCompletionReason::None;

    UPROPERTY(Transient)
    EKashmirMovementDeliveryCompletionReason LastCompletionReason =
        EKashmirMovementDeliveryCompletionReason::None;
};
