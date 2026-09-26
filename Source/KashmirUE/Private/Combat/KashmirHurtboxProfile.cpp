#include "Combat/KashmirHurtboxProfile.h"


bool FKashmirHurtboxDefinition::IsValid(
    FString& OutReason) const
{
    OutReason.Reset();

    if (Id.IsNone())
    {
        OutReason =
            TEXT("hurtbox id must not be empty");

        return false;
    }

    if (BoneName.IsNone())
    {
        OutReason =
            TEXT("hurtbox bone name must not be empty");

        return false;
    }

    if (!HitRegion.IsValid())
    {
        OutReason =
            TEXT("hurtbox hit region must be valid");

        return false;
    }

    if (
        !FMath::IsFinite(LocalOffset.X) ||
        !FMath::IsFinite(LocalOffset.Y) ||
        !FMath::IsFinite(LocalOffset.Z))
    {
        OutReason =
            TEXT("hurtbox local offset must be finite");

        return false;
    }

    switch (Shape)
    {
        case EKashmirHurtboxShape::Box:
        {
            if (
                !FMath::IsFinite(BoxHalfExtent.X) ||
                !FMath::IsFinite(BoxHalfExtent.Y) ||
                !FMath::IsFinite(BoxHalfExtent.Z) ||
                BoxHalfExtent.X <= 0.0f ||
                BoxHalfExtent.Y <= 0.0f ||
                BoxHalfExtent.Z <= 0.0f)
            {
                OutReason =
                    TEXT(
                        "box hurtbox half extents must be finite and positive"
                    );

                return false;
            }

            break;
        }

        case EKashmirHurtboxShape::Capsule:
        {
            if (
                !FMath::IsFinite(Radius) ||
                !FMath::IsFinite(HalfHeight) ||
                Radius <= 0.0f ||
                HalfHeight <= 0.0f ||
                HalfHeight < Radius)
            {
                OutReason =
                    TEXT(
                        "capsule radius and half height must be valid"
                    );

                return false;
            }

            break;
        }

        case EKashmirHurtboxShape::Sphere:
        {
            if (
                !FMath::IsFinite(Radius) ||
                Radius <= 0.0f)
            {
                OutReason =
                    TEXT(
                        "sphere radius must be finite and positive"
                    );

                return false;
            }

            break;
        }
    }

    return true;
}


bool UKashmirHurtboxProfile::ValidateProfile(
    FString& OutReason) const
{
    OutReason.Reset();

    if (Hurtboxes.IsEmpty())
    {
        OutReason =
            TEXT("hurtbox profile must contain at least one hurtbox");

        return false;
    }

    TSet<FName> SeenIds;

    for (const FKashmirHurtboxDefinition& Hurtbox :
        Hurtboxes)
    {
        FString DefinitionReason;

        if (!Hurtbox.IsValid(
                DefinitionReason))
        {
            OutReason =
                FString::Printf(
                    TEXT(
                        "invalid hurtbox '%s': %s"
                    ),
                    *Hurtbox.Id.ToString(),
                    *DefinitionReason
                );

            return false;
        }

        if (SeenIds.Contains(
                Hurtbox.Id))
        {
            OutReason =
                FString::Printf(
                    TEXT(
                        "duplicate hurtbox id '%s'"
                    ),
                    *Hurtbox.Id.ToString()
                );

            return false;
        }

        SeenIds.Add(
            Hurtbox.Id
        );
    }

    return true;
}


const FKashmirHurtboxDefinition*
UKashmirHurtboxProfile::FindById(
    FName Id) const
{
    for (const FKashmirHurtboxDefinition& Hurtbox :
        Hurtboxes)
    {
        if (Hurtbox.Id == Id)
        {
            return &Hurtbox;
        }
    }

    return nullptr;
}