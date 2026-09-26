#include "Combat/KashmirHurtboxComponent.h"

#include "Combat/KashmirHurtboxProfile.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

#include "GameFramework/Actor.h"


UKashmirHurtboxComponent::UKashmirHurtboxComponent()
{
    PrimaryComponentTick.bCanEverTick =
        false;
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