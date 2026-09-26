#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirHitEvidence.h"

#include "KashmirClashResolver.generated.h"


UENUM(BlueprintType)
enum class EKashmirClashOutcome : uint8
{
    NoClash,
    Clash
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirClashInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirHitEvidence FirstEvidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirHitEvidence SecondEvidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFirstAttackActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSecondAttackActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MinimumRelativeSpeed = 0.0f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirClashResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bClashed = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirClashOutcome Outcome = EKashmirClashOutcome::NoClash;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector RelativeVelocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RelativeSpeed = 0.0f;
};


class KASHMIRUE_API FKashmirClashResolver
{
public:
    bool Resolve(
        const FKashmirClashInput& Input,
        FKashmirClashResult& OutResult,
        FString& OutReason
    ) const;
};
