#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirWeaponTraceComponent.h"
#include "Runtime/KashmirActionRuntime.h"

#include "KashmirDirectionalSwordComponent.generated.h"


class UKashmirSwordPresentationComponent;
class UKashmirMovementDeliveryComponent;


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirDirectionalSwordContact
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bResolved = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirWeaponTraceHit TraceHit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName ActionId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirActionRequest ActionRequest;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirCombatActionDefinition CombatDefinition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirMeleeArchetype Archetype =
        EKashmirMeleeArchetype::OneHandedSword;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName StyleId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName SourceId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirContactSourceType ContactSource =
        EKashmirContactSourceType::Weapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BaseDamage = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AttackPowerMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float BaseGuardDamage = 0.0f;
};


class KASHMIRUE_API FKashmirDirectionalSwordContactResolver
{
public:
    bool Resolve(
        const FKashmirWeaponTraceHit& TraceHit,
        const FKashmirSwordActionPlan& Plan,
        FKashmirDirectionalSwordContact& OutContact,
        FString& OutReason) const;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FKashmirDirectionalSwordContactSignature,
    const FKashmirDirectionalSwordContact&,
    Contact);


UENUM(BlueprintType)
enum class EKashmirPendingTechniqueClearReason : uint8
{
    None,
    Consumed,
    Expired,
    SourceEnded,
    SourceChanged,
    Superseded,
    Invalid,
    WindowMissed,
    Cancelled,
    ResourceFailure
};


struct FKashmirPendingTechniqueRequestState
{
    FKashmirTechniqueRequest Request;
    TWeakObjectPtr<const UKashmirWeaponCombatStyle> Style;
    FName TechniqueId;
    FName SourceActionId;
    FName SourceTechniqueId;
    uint64 SourceExecutionSerial = 0;
    float BufferedAtActionElapsed = 0.0f;
    float Age = 0.0f;
    float Lifetime = 0.0f;
};


/**
 * Migration adapter for the authored sword prototype. Technique requests are
 * the product authority; legacy gesture entry points remain only until their
 * tests and assets can be retired safely.
 */
UCLASS(ClassGroup=(Kashmir), meta=(BlueprintSpawnableComponent))
class KASHMIRUE_API UKashmirDirectionalSwordComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKashmirDirectionalSwordComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    void SetProfile(UKashmirDirectionalSwordProfile* InProfile);

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    void SetPresentationComponent(
        UKashmirSwordPresentationComponent* InPresentationComponent);

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    void SetWeaponTraceComponent(
        UKashmirWeaponTraceComponent* InWeaponTraceComponent);

    void SetMovementDeliveryComponent(
        UKashmirMovementDeliveryComponent* InMovementDeliveryComponent);

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    bool BeginGesture(FString& OutReason);

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    bool AddGestureDelta(
        FVector2D Delta,
        float DeltaSeconds,
        FString& OutReason);

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    bool CompleteGesture(FString& OutReason);

    /** Starts an authored action from AI/replay/network without gesture capture. */
    UFUNCTION(BlueprintCallable, Category="Combat|Action")
    bool StartActionRequest(
        const FKashmirActionRequest& Request,
        FString& OutReason);

    /** Resolves a logical slot through weapon-style data, then reuses ActionRuntime. */
    UFUNCTION(BlueprintCallable, Category="Combat|Technique")
    bool StartTechniqueRequest(
        const FKashmirTechniqueRequest& Request,
        const UKashmirWeaponCombatStyle* Style,
        FString& OutReason);

    /** Cancels the current Action through ActionRuntime cancellability rules. */
    UFUNCTION(BlueprintCallable, Category="Combat|Action")
    bool CancelCurrentAction(FString& OutReason);

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    void CancelGesture();

    /** Advances the authoritative timeline independently of component ticking. */
    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    bool AdvanceRuntime(float DeltaSeconds, FString& OutReason);

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    bool IsCapturingGesture() const
    {
        return bCapturingGesture;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    UKashmirDirectionalSwordProfile* GetProfile() const
    {
        return Profile;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    FKashmirActionRuntimeState GetRuntimeState() const;

    /** Consumes authoritative runtime events for orchestration/tests. */
    TArray<FKashmirActionEvent> DrainRuntimeEvents();

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    FKashmirSwordActionPlan GetActivePlan() const
    {
        return ActivePlan;
    }

    /** Last Technique dispatch rejection for Blueprint, Python and Aura diagnostics. */
    UFUNCTION(BlueprintPure, Category="Combat|Technique")
    FString GetLastTechniqueRequestReason() const
    {
        return LastTechniqueRequestReason;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    bool HasPendingTechniqueRequest() const
    {
        return PendingTechniqueRequest.IsSet();
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    EKashmirTechniqueSlot GetPendingTechniqueSlot() const
    {
        return PendingTechniqueRequest.IsSet()
            ? PendingTechniqueRequest->Request.Slot
            : EKashmirTechniqueSlot::None;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    FName GetPendingTechniqueId() const
    {
        return PendingTechniqueRequest.IsSet()
            ? PendingTechniqueRequest->TechniqueId
            : NAME_None;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    EKashmirTechniqueRequestSource GetPendingTechniqueSource() const
    {
        return PendingTechniqueRequest.IsSet()
            ? PendingTechniqueRequest->Request.Source
            : EKashmirTechniqueRequestSource::Player;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    float GetPendingTechniqueAge() const
    {
        return PendingTechniqueRequest.IsSet() ? PendingTechniqueRequest->Age : 0.0f;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    float GetPendingTechniqueBufferedAtActionElapsed() const
    {
        return PendingTechniqueRequest.IsSet()
            ? PendingTechniqueRequest->BufferedAtActionElapsed
            : 0.0f;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    float GetPendingTechniqueLifetime() const
    {
        return PendingTechniqueRequest.IsSet()
            ? PendingTechniqueRequest->Lifetime
            : TechniqueRequestBufferLifetime;
    }

    UFUNCTION(BlueprintPure, Category="Combat|Technique|Debug", meta=(DevelopmentOnly))
    EKashmirPendingTechniqueClearReason GetLastPendingTechniqueClearReason() const
    {
        return LastPendingTechniqueClearReason;
    }

    UPROPERTY(BlueprintAssignable, Category="Combat|Directional Sword")
    FKashmirDirectionalSwordContactSignature OnSwordContact;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Directional Sword")
    TObjectPtr<UKashmirDirectionalSwordProfile> Profile;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Directional Sword")
    TArray<FKashmirResourcePool> InitialResources;

    /** Single-intent early input buffer. It never widens an authored transition window. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Technique Buffer",
        meta=(ClampMin="0.0", Units="s"))
    float TechniqueRequestBufferLifetime = 0.20f;

private:
    enum class ETechniqueBufferAttempt : uint8
    {
        NotApplicable,
        Buffered,
        Rejected
    };

    bool RebuildRuntime(FString& OutReason);
    bool ResolveTechniqueRequestPlan(
        const FKashmirTechniqueRequest& Request,
        const UKashmirWeaponCombatStyle* Style,
        FKashmirTechniqueActionPlan& OutTechniquePlan,
        FKashmirSwordActionPlan& OutPlan,
        FString& OutReason) const;
    ETechniqueBufferAttempt TryBufferTechniqueRequest(
        const FKashmirTechniqueRequest& Request,
        const UKashmirWeaponCombatStyle* Style,
        const FKashmirTechniqueActionPlan& TechniquePlan,
        const FKashmirActionRuntimeState& CurrentState,
        FString& OutReason);
    bool FindFutureTechniqueTransition(
        const UKashmirWeaponCombatStyle* Style,
        FName FromTechniqueId,
        FName ToTechniqueId,
        float Elapsed,
        float MaximumWait,
        float SourceActionLifetime,
        const FGameplayTagContainer& ContextTags,
        FKashmirTechniqueTransitionRule& OutRule,
        bool& bOutRelevantRuleExists,
        bool& bOutWindowMissed) const;
    void UpdatePendingTechniqueRequest(float DeltaSeconds);
    void ClearPendingTechniqueRequest(EKashmirPendingTechniqueClearReason Reason);
    bool StartResolvedPlan(
        const FKashmirSwordActionPlan& Plan,
        FString& OutReason);
    bool ValidateResolvedPlan(
        const FKashmirSwordActionPlan& Plan,
        FString& OutReason) const;
    bool TryTransitionTechnique(
        const FKashmirSwordActionPlan& Plan,
        const FKashmirTechniqueTransitionRule& TechniqueRule,
        const FGameplayTagContainer& ContextTags,
        FString& OutReason);
    void ResetTraceForTransition(
        const FKashmirActionRuntimeState& DestinationState);
    void ApplyPresentation(const FKashmirActionRuntimeState& RuntimeState);
    void SynchronizeTraceWindow(
        const FKashmirActionRuntimeState& PreviousState,
        const FKashmirActionRuntimeState& CurrentState);
    bool SampleWeaponTrace(float DeltaSeconds, FString& OutReason);

    UPROPERTY(Transient)
    TObjectPtr<UKashmirSwordPresentationComponent> PresentationComponent;

    UPROPERTY(Transient)
    TObjectPtr<UKashmirWeaponTraceComponent> WeaponTraceComponent;

    UPROPERTY(Transient)
    TObjectPtr<UKashmirMovementDeliveryComponent> MovementDeliveryComponent;

    FKashmirSwordGestureInput CapturedGesture;
    FVector2D AccumulatedPosition = FVector2D::ZeroVector;
    bool bCapturingGesture = false;

    FKashmirSwordActionPlan ActivePlan;
    TOptional<FKashmirPendingTechniqueRequestState> PendingTechniqueRequest;
    uint64 ActionExecutionSerial = 0;
    UPROPERTY(Transient)
    EKashmirPendingTechniqueClearReason LastPendingTechniqueClearReason =
        EKashmirPendingTechniqueClearReason::None;
    UPROPERTY(Transient)
    FString LastTechniqueRequestReason;
    TUniquePtr<FKashmirResourceRuntime> Resources;
    TUniquePtr<FKashmirActionRuntime> Runtime;
};
