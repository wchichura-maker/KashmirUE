#pragma once

#include "CoreMinimal.h"
#include "Contracts/KashmirActionContracts.h"
#include "KashmirEffectApplier.generated.h"

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirHealthState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Current = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Maximum = 100.0f;

    bool IsValid(FString& OutReason) const;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirEffectApplicationResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bApplied = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float HealthBefore = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float HealthAfter = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AppliedMagnitude = 0.0f;
};

class KASHMIRUE_API FKashmirEffectApplier
{
public:

    bool Apply(
        const FKashmirEffectResult& Effect,
        FKashmirHealthState& Health,
        FKashmirEffectApplicationResult& OutResult,
        FString& OutReason
    ) const;
};