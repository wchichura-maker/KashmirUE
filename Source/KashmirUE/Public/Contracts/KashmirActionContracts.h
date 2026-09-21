#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KashmirActionContracts.generated.h"

UENUM(BlueprintType)
enum class EKashmirDeliveryType : uint8
{
    Contact, Projectile, Beam, Area, Field, Grapple, Self, Target
};

UENUM(BlueprintType)
enum class EKashmirTargetType : uint8
{
    Self, Ally, Enemy, Entity, Point, Area, Direction, Object, Environment
};

UENUM(BlueprintType)
enum class EKashmirResolutionType : uint8
{
    Damage, Heal, Control, Modify, Displace, Create, Destroy, Information
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirActionRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D Direction = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    bool IsValid(FString& OutReason) const;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatActionDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirDeliveryType Delivery = EKashmirDeliveryType::Contact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKashmirTargetType Target = EKashmirTargetType::Enemy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EKashmirResolutionType> Effects;
};

USTRUCT(BlueprintType)
struct KASHMIRUE_API FKashmirCombatResult
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bDelivered = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FName> TargetIds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FGameplayTagContainer EffectTags;
};
