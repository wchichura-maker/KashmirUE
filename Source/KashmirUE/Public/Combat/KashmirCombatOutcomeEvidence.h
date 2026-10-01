#pragma once

#include "CoreMinimal.h"
#include "Contracts/KashmirActionContracts.h"

#include "KashmirCombatOutcomeEvidence.generated.h"


UENUM(BlueprintType)
enum class EKashmirCombatOutcomeFinalizationReason : uint8
{
    None,
    Completed,
    Interrupted,
    Transitioned
};


UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EKashmirCombatOutcomeFact : uint8
{
    None = 0,
    HadContact = 1 << 0,
    AppliedDamage = 1 << 1,
    Blocked = 1 << 2,
    Parried = 1 << 3,
    GuardBroken = 1 << 4
};
ENUM_CLASS_FLAGS(EKashmirCombatOutcomeFact);


/** One authoritative, fully processed combat contact observed by an execution. */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatOutcomeContact
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName TargetId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(ScriptName="did_apply_damage"))
    bool bDamageApplied = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(ScriptName="applied_damage_amount"))
    float DamageApplied = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bBlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bParried = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bGuardBroken = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirCombatResult CombatResult;
};


/** Compact evidence accumulated for exactly one started Action/Technique execution. */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatExecutionOutcome
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int64 ExecutionSerial = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName TechniqueId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName ActionId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 ContactCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 UniqueTargetCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bHadContact = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bAppliedDamage = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWasBlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWasParried = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bCausedGuardBreak = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float TotalDamageApplied = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName LastTargetId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FKashmirCombatResult LastCombatResult;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bFinalized = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirCombatOutcomeFinalizationReason FinalizationReason =
        EKashmirCombatOutcomeFinalizationReason::None;
};


/** Minimal immutable projection consumed by Technique transition grammar. */
USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatOutcomeFacts
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bHadContact = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bAppliedDamage = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bBlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bParried = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bGuardBroken = false;

    static FKashmirCombatOutcomeFacts FromOutcome(
        const FKashmirCombatExecutionOutcome& Outcome);
    static bool IsValidRequirementMask(int32 RequiredFacts);
    bool Satisfies(int32 RequiredFacts) const;
};


/** Generic execution-scoped accumulator. It observes results and owns no combat decisions. */
class KASHMIRUE_API FKashmirCombatOutcomeAccumulator
{
public:
    void BeginExecution(int64 ExecutionSerial, FName TechniqueId, FName ActionId);
    bool RecordContact(const FKashmirCombatOutcomeContact& Contact, FString& OutReason);
    void Finalize(EKashmirCombatOutcomeFinalizationReason Reason);
    void Reset();

    bool HasCurrent() const { return bHasCurrent; }
    bool HasLastFinalized() const { return bHasLastFinalized; }
    const FKashmirCombatExecutionOutcome& GetCurrent() const { return Current; }
    const FKashmirCombatExecutionOutcome& GetLastFinalized() const
    {
        return LastFinalized;
    }

private:
    FKashmirCombatExecutionOutcome Current;
    FKashmirCombatExecutionOutcome LastFinalized;
    TSet<FName> UniqueTargets;
    bool bHasCurrent = false;
    bool bHasLastFinalized = false;
};
