#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirSwordPoseResolver.h"

#include "KashmirSwordPresentationComponent.generated.h"


class USkeletalMeshComponent;


UENUM(BlueprintType)
enum class EKashmirSwordPresentationCommand : uint8
{
    None,
    Play,
    Synchronize,
    Stop
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordPresentationState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FName ActionId;
};


USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirSwordPresentationSyncResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EKashmirSwordPresentationCommand Command = EKashmirSwordPresentationCommand::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float MontagePositionSeconds = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float PlayRate = 1.0f;
};


class KASHMIRUE_API FKashmirSwordPresentationSyncResolver
{
public:

    bool Resolve(
        const FKashmirSwordActionPlan& Plan,
        const FKashmirActionRuntimeState& RuntimeState,
        const FKashmirSwordPresentationState& PresentationState,
        FKashmirSwordPresentationSyncResult& OutResult,
        FString& OutReason
    ) const;
};


UCLASS(ClassGroup=(Kashmir), meta=(BlueprintSpawnableComponent))
class KASHMIRUE_API UKashmirSwordPresentationComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UKashmirSwordPresentationComponent();

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    void SetSkeletalMesh(USkeletalMeshComponent* InSkeletalMesh);

    /** Applies presentation for an authoritative runtime snapshot. */
    bool ApplyRuntimeState(
        const FKashmirSwordActionPlan& Plan,
        const FKashmirActionRuntimeState& RuntimeState,
        FString& OutReason
    );

    UFUNCTION(BlueprintCallable, Category="Combat|Directional Sword")
    void StopPresentation(float BlendOutSeconds = 0.10f);

    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    FKashmirSwordPresentationState GetPresentationState() const
    {
        return PresentationState;
    }

    /** Parameters for the Animation Blueprint / Control Rig presentation layer. */
    UFUNCTION(BlueprintPure, Category="Combat|Directional Sword")
    FKashmirSwordPoseResult GetCurrentPose() const
    {
        return CurrentPose;
    }

private:

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(Transient)
    TObjectPtr<UAnimMontage> ActiveMontage;

    FKashmirSwordPresentationState PresentationState;

    FKashmirSwordPoseResult CurrentPose;
};
