#include "Combat/KashmirWeaponContact.h"

namespace
{
    bool IsFiniteVector(
        const FVector& Vector)
    {
        return
            FMath::IsFinite(Vector.X) &&
            FMath::IsFinite(Vector.Y) &&
            FMath::IsFinite(Vector.Z);
    }
}

bool FKashmirWeaponContactPoint::IsValid(
    FString& OutReason) const
{
    if (Id.IsNone())
    {
        OutReason =
            TEXT("weapon contact point id is required");

        return false;
    }

    if (!IsFiniteVector(Position))
    {
        OutReason =
            TEXT("weapon contact point position must be finite");

        return false;
    }

    return true;
}

bool FKashmirWeaponContactFrame::IsValid(
    FString& OutReason) const
{
    if (Points.IsEmpty())
    {
        OutReason =
            TEXT("weapon contact frame requires at least one point");

        return false;
    }

    TSet<FName> SeenIds;

    for (const FKashmirWeaponContactPoint& Point :
        Points)
    {
        if (!Point.IsValid(OutReason))
        {
            return false;
        }

        if (SeenIds.Contains(Point.Id))
        {
            OutReason =
                TEXT("weapon contact point ids must be unique");

            return false;
        }

        SeenIds.Add(Point.Id);
    }

    return true;
}

const FKashmirWeaponContactPoint*
FKashmirWeaponContactFrame::FindPoint(
    const FName PointId) const
{
    for (const FKashmirWeaponContactPoint& Point :
        Points)
    {
        if (Point.Id == PointId)
        {
            return &Point;
        }
    }

    return nullptr;
}

bool FKashmirWeaponContactBuilder::BuildSweepSegments(
    const FKashmirWeaponContactFrame& PreviousFrame,
    const FKashmirWeaponContactFrame& CurrentFrame,
    TArray<FKashmirWeaponSweepSegment>& OutSegments,
    FString& OutReason)
{
    OutSegments.Reset();
    OutReason.Reset();

    if (!PreviousFrame.IsValid(OutReason))
    {
        return false;
    }

    if (!CurrentFrame.IsValid(OutReason))
    {
        return false;
    }

    for (const FKashmirWeaponContactPoint& CurrentPoint :
        CurrentFrame.Points)
    {
        const FKashmirWeaponContactPoint* PreviousPoint =
            PreviousFrame.FindPoint(
                CurrentPoint.Id
            );

        if (PreviousPoint == nullptr)
        {
            OutReason =
                TEXT("weapon contact frame point sets do not match");

            OutSegments.Reset();
            return false;
        }

        FKashmirWeaponSweepSegment Segment;

        Segment.PointId =
            CurrentPoint.Id;

        Segment.PreviousPosition =
            PreviousPoint->Position;

        Segment.CurrentPosition =
            CurrentPoint.Position;

        Segment.Delta =
            Segment.CurrentPosition -
            Segment.PreviousPosition;

        Segment.Distance =
            Segment.Delta.Size();

        OutSegments.Add(Segment);
    }

    if (PreviousFrame.Points.Num() !=
        CurrentFrame.Points.Num())
    {
        OutReason =
            TEXT("weapon contact frame point sets do not match");

        OutSegments.Reset();
        return false;
    }

    return true;
}