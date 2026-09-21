#pragma once

#include "CoreMinimal.h"
#include "Contracts/KashmirActionContracts.h"
#include "GameplayTagContainer.h"
#include "KashmirActionRuntime.generated.h"

UENUM(BlueprintType) enum class EKashmirActionPhase : uint8 { Inactive, Startup, Active, Recovery, Complete, Interrupted };
UENUM(BlueprintType) enum class EKashmirActionEventType : uint8 { Started, Completed, Interrupted, Transitioned };

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirResourcePool { GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag ResourceTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Current = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Maximum = 0.0f;
    bool IsValid(FString& OutReason) const;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirResourceCost { GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag ResourceTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Amount = 0.0f;
    bool IsValid(FString& OutReason) const;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirResourceChange { GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FGameplayTag ResourceTag;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Before = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float After = 0.0f;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirResourcePaymentResult { GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bPaid = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FKashmirResourceChange> Changes;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<FString> Errors;
};
class KASHMIRUE_API FKashmirResourceRuntime {
public:
    explicit FKashmirResourceRuntime(const TArray<FKashmirResourcePool>& InPools);
    bool CanPay(const TArray<FKashmirResourceCost>& Costs, FKashmirResourcePaymentResult& OutResult) const;
    bool Pay(const TArray<FKashmirResourceCost>& Costs, FKashmirResourcePaymentResult& OutResult);
    bool Regenerate(const FGameplayTag& ResourceTag, float DeltaSeconds, float PerSecond, FString& OutReason);
    float GetCurrent(const FGameplayTag& ResourceTag) const;
private: TMap<FGameplayTag, FKashmirResourcePool> Pools;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirActionDefinition { GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActionId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float StartupDuration = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ActiveDuration = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RecoveryDuration = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCancellable = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<EKashmirActionPhase> CancelWindows;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FKashmirResourceCost> StartCosts;
    bool IsValid(FString& OutReason) const; float GetTotalDuration() const;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirTransitionRule { GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName FromActionId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ToActionId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinElapsed = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxElapsed = -1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer RequiredTags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer BlockedTags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Priority = 0;
    bool IsValid(FString& OutReason) const;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirActionRuntimeState { GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName ActionId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EKashmirActionPhase Phase = EKashmirActionPhase::Inactive;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Elapsed = 0.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bActive = false;
};
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirActionEvent { GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName EventId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EKashmirActionEventType Type = EKashmirActionEventType::Started;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName ActionId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Timestamp = 0.0f;
};
class KASHMIRUE_API FKashmirActionRuntime {
public:
    FKashmirActionRuntime(const TMap<FName, FKashmirActionDefinition>& InDefinitions, const TArray<FKashmirTransitionRule>& InTransitions, FKashmirResourceRuntime& InResources);
    bool Start(const FKashmirActionRequest& Request, FString& OutReason);
    bool Advance(float DeltaSeconds, FString& OutReason);
    bool TryCancel(const FKashmirActionRequest& NextRequest, FString& OutReason);
    bool TransitionTo(const FKashmirActionRequest& NextRequest, const FGameplayTagContainer& ContextTags, FString& OutReason);
    TArray<FName> GetTransitionOptions(const FGameplayTagContainer& ContextTags) const;
    FKashmirActionRuntimeState GetState() const;
    TArray<FKashmirActionEvent> DrainEvents();
private:
    bool Validate(const FKashmirActionRequest& Request, const FKashmirActionDefinition*& OutDefinition, FString& OutReason) const;
    bool PayAndBegin(const FKashmirActionRequest& Request, const FKashmirActionDefinition& Definition, FString& OutReason);
    void Begin(const FKashmirActionDefinition& Definition); void AppendEvent(EKashmirActionEventType Type, FName ActionId); EKashmirActionPhase ResolvePhase() const;
    TMap<FName, FKashmirActionDefinition> Definitions; TArray<FKashmirTransitionRule> Transitions; FKashmirResourceRuntime& Resources;
    FName ActiveActionId; float Elapsed = 0.0f; bool bActive = false; bool bInterrupted = false; bool bCompleted = false; int32 NextEventSequence = 1; TArray<FKashmirActionEvent> Events;
};
