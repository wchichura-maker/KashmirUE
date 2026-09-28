#pragma once

#include "CoreMinimal.h"

#include "Combat/KashmirHitEvidence.h"
#include "Combat/KashmirStaggerResolver.h"

#include "KashmirPhysicalReactionResolver.generated.h"


UENUM(BlueprintType)
enum class EKashmirPhysicalReactionMode : uint8
{
    None,
    PartialBody,
    RagdollCandidate
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirPhysicalReactionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float MaximumIntensity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PartialBodyBlendWeight = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float RecoveryDurationSeconds = 0.35f;

    /** Allows only a logical candidate result; this resolver never activates ragdoll. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowRagdollCandidate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float RagdollCandidateThreshold = 800.0f;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirPhysicalReactionInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirHitEvidence Evidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirStaggerResult Stagger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKashmirPhysicalReactionConfig Config;

    bool IsValid(FString& OutReason) const;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirPhysicalReactionResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bReactionRequested = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirPhysicalReactionMode Mode = EKashmirPhysicalReactionMode::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirStaggerTarget Target = EKashmirStaggerTarget::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Direction = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Intensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PartialBodyBlendWeight = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RecoveryDurationSeconds = 0.0f;

    /** Region is meaningful only when the defender is the reaction target. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FGameplayTag HitRegion;
};


class KASHMIRUE_API FKashmirPhysicalReactionResolver
{
public:

    bool Resolve(
        const FKashmirPhysicalReactionInput& Input,
        FKashmirPhysicalReactionResult& OutResult,
        FString& OutReason
    ) const;
};
