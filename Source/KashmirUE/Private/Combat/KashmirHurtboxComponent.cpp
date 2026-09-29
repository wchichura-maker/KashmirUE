#include "Combat/KashmirHurtboxComponent.h"

#include "Combat/KashmirHurtboxProfile.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"


UKashmirHurtboxComponent::UKashmirHurtboxComponent()
{
    PrimaryComponentTick.bCanEverTick =
        false;

    /*
     * The Manny baseline profile is the project default combat
     * anatomy, so a spawned actor only has to add the component.
     */
    static ConstructorHelpers::FObjectFinder<UKashmirHurtboxProfile>
        DefaultProfile(
            TEXT("/Game/KashmirAct/Combat/Hurtboxes/")
            TEXT("DA_KashmirHurtbox_Manny.")
            TEXT("DA_KashmirHurtbox_Manny"));

    if (DefaultProfile.Succeeded())
    {
        HurtboxProfile =
            DefaultProfile.Object;
    }
}


void UKashmirHurtboxComponent::BeginPlay()
{
    Super::BeginPlay();

    if (HurtboxProfile == nullptr)
    {
        return;
    }

    FString Reason;

    if (!BuildFromProfile(
            Reason))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Hurtbox build failed: %s"), *Reason);
    }
}


bool UKashmirHurtboxComponent::BuildFromProfile(
    FString& OutReason)
{
    OutReason.Reset();

    USkeletalMeshComponent* Mesh =
        TargetMesh;

    if (Mesh == nullptr)
    {
        const AActor* OwnerActor =
            GetOwner();

        if (OwnerActor != nullptr)
        {
            Mesh =
                OwnerActor->FindComponentByClass<
                    USkeletalMeshComponent>();
        }
    }

    if (Mesh == nullptr)
    {
        OutReason =
            TEXT(
                "hurtbox component has no skeletal mesh to build against"
            );

        return false;
    }

    return Build(
        Mesh,
        HurtboxProfile,
        OutReason
    );
}


bool UKashmirHurtboxComponent::Build(
    USkeletalMeshComponent* InMesh,
    const UKashmirHurtboxProfile* InProfile,
    FString& OutReason)
{
    OutReason.Reset();

    ClearHurtboxes();

    if (InMesh == nullptr)
    {
        OutReason =
            TEXT("hurtbox mesh must not be null");

        return false;
    }

    if (InProfile == nullptr)
    {
        OutReason =
            TEXT("hurtbox profile must not be null");

        return false;
    }

    FString ProfileReason;

    if (!InProfile->ValidateProfile(
            ProfileReason))
    {
        OutReason =
            FString::Printf(
                TEXT(
                    "invalid hurtbox profile: %s"
                ),
                *ProfileReason
            );

        return false;
    }

    AActor* Owner =
        GetOwner();

    if (Owner == nullptr)
    {
        OutReason =
            TEXT(
                "hurtbox component requires an owner"
            );

        return false;
    }

    /*
     * Validate every required bone before
     * creating any collision component.
     *
     * Build is therefore atomic:
     * either the whole profile is valid for
     * this skeleton, or nothing is created.
     */
    for (const FKashmirHurtboxDefinition& Definition :
        InProfile->Hurtboxes)
    {
        if (InMesh->GetBoneIndex(
                Definition.BoneName) ==
            INDEX_NONE)
        {
            OutReason =
                FString::Printf(
                    TEXT(
                        "hurtbox '%s' references missing bone '%s'"
                    ),
                    *Definition.Id.ToString(),
                    *Definition.BoneName.ToString()
                );

            return false;
        }
    }
    UWorld* OwnerWorld =
        Owner->GetWorld();

    if (OwnerWorld == nullptr)
    {
        OutReason =
            TEXT(
                "hurtbox component owner must belong to a valid world"
            );

        return false;
    }
    BoundMesh =
        InMesh;

    BoundProfile =
        InProfile;

    for (const FKashmirHurtboxDefinition& Definition :
        InProfile->Hurtboxes)
    {
        UPrimitiveComponent* NewComponent =
            nullptr;

        switch (Definition.Shape)
        {
            case EKashmirHurtboxShape::Box:
            {
                UBoxComponent* Box =
                    NewObject<UBoxComponent>(
                        Owner,
                        NAME_None,
                        RF_Transient
                    );

                Box->SetBoxExtent(
                    Definition.BoxHalfExtent,
                    false
                );

                NewComponent =
                    Box;

                break;
            }

            case EKashmirHurtboxShape::Capsule:
            {
                UCapsuleComponent* Capsule =
                    NewObject<UCapsuleComponent>(
                        Owner,
                        NAME_None,
                        RF_Transient
                    );

                Capsule->SetCapsuleSize(
                    Definition.Radius,
                    Definition.HalfHeight,
                    false
                );

                NewComponent =
                    Capsule;

                break;
            }

            case EKashmirHurtboxShape::Sphere:
            {
                USphereComponent* Sphere =
                    NewObject<USphereComponent>(
                        Owner,
                        NAME_None,
                        RF_Transient
                    );

                Sphere->SetSphereRadius(
                    Definition.Radius,
                    false
                );

                NewComponent =
                    Sphere;

                break;
            }
        }

        if (NewComponent == nullptr)
        {
            OutReason =
                FString::Printf(
                    TEXT(
                        "failed to create hurtbox '%s'"
                    ),
                    *Definition.Id.ToString()
                );

            ClearHurtboxes();

            return false;
        }

        /*
         * Hurtboxes participate only in
         * combat queries.
         */
        NewComponent->SetCollisionEnabled(
            ECollisionEnabled::QueryOnly
        );

        NewComponent->SetCollisionObjectType(
            ECC_WorldDynamic
        );

        NewComponent->SetCollisionResponseToAllChannels(
            ECR_Ignore
        );

        NewComponent->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1,
            ECR_Block
        );

        NewComponent->SetGenerateOverlapEvents(
            false
        );

        Owner->AddInstanceComponent(
            NewComponent
        );

        NewComponent->RegisterComponent();

        const bool bAttached =
            NewComponent->AttachToComponent(
                InMesh,
                FAttachmentTransformRules::
                    SnapToTargetNotIncludingScale,
                Definition.BoneName
            );

        if (!bAttached)
        {
            OutReason =
                FString::Printf(
                    TEXT(
                        "failed to attach hurtbox '%s' to bone '%s'"
                    ),
                    *Definition.Id.ToString(),
                    *Definition.BoneName.ToString()
                );

            ClearHurtboxes();

            return false;
        }

        NewComponent->SetRelativeLocation(
            Definition.LocalOffset
        );

        NewComponent->SetRelativeRotation(
            Definition.LocalRotation
        );

        NewComponent->ComponentTags.Add(
            Definition.Id
        );

        HurtboxComponents.Add(
            NewComponent
        );
    }

    return true;
}


void UKashmirHurtboxComponent::ClearHurtboxes()
{
    for (UPrimitiveComponent* Component :
        HurtboxComponents)
    {
        if (Component == nullptr)
        {
            continue;
        }

        Component->DestroyComponent();
    }

    HurtboxComponents.Reset();

    BoundMesh =
        nullptr;

    BoundProfile =
        nullptr;
}


int32 UKashmirHurtboxComponent::GetHurtboxCount() const
{
    return HurtboxComponents.Num();
}


bool UKashmirHurtboxComponent::ResolveHitComponent(
    const UPrimitiveComponent* HitComponent,
    FName& OutHurtboxId,
    FGameplayTag& OutHitRegion) const
{
    OutHurtboxId =
        NAME_None;

    OutHitRegion =
        FGameplayTag();

    if (
        HitComponent == nullptr ||
        BoundProfile == nullptr)
    {
        return false;
    }

    const int32 Index =
        HurtboxComponents.IndexOfByPredicate(
            [HitComponent](
                const TObjectPtr<UPrimitiveComponent>& Candidate)
            {
                return Candidate.Get() ==
                    HitComponent;
            }
        );

    if (
        Index == INDEX_NONE ||
        !BoundProfile->Hurtboxes.IsValidIndex(
            Index))
    {
        return false;
    }

    const FKashmirHurtboxDefinition& Definition =
        BoundProfile->Hurtboxes[Index];

    OutHurtboxId =
        Definition.Id;

    OutHitRegion =
        Definition.HitRegion;

    return true;
}


const FKashmirHurtboxDefinition*
UKashmirHurtboxComponent::FindDefinitionForComponent(
    const UPrimitiveComponent* HitComponent) const
{
    if (
        HitComponent == nullptr ||
        BoundProfile == nullptr)
    {
        return nullptr;
    }

    const int32 Index =
        HurtboxComponents.IndexOfByPredicate(
            [HitComponent](
                const TObjectPtr<UPrimitiveComponent>& Candidate)
            {
                return Candidate.Get() ==
                    HitComponent;
            }
        );

    if (
        Index == INDEX_NONE ||
        !BoundProfile->Hurtboxes.IsValidIndex(
            Index))
    {
        return nullptr;
    }

    return &BoundProfile->Hurtboxes[Index];
}


TArray<UPrimitiveComponent*>
UKashmirHurtboxComponent::GetHurtboxPrimitives() const
{
    TArray<UPrimitiveComponent*> Result;

    Result.Reserve(
        HurtboxComponents.Num()
    );

    for (const TObjectPtr<UPrimitiveComponent>& Component :
        HurtboxComponents)
    {
        Result.Add(
            Component.Get()
        );
    }

    return Result;
}