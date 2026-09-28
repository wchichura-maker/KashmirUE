#include "Combat/KashmirDirectionalSwordResolver.h"


namespace
{
    bool IsFiniteVector2D(const FVector2D& Value)
    {
        return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y);
    }

    EKashmirSwordGestureDirection QuantizeDirection(const FVector2D& Direction)
    {
        float Degrees = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
        if (Degrees < 0.0f)
        {
            Degrees += 360.0f;
        }

        const int32 Sector = FMath::FloorToInt((Degrees + 22.5f) / 45.0f) % 8;
        return static_cast<EKashmirSwordGestureDirection>(Sector);
    }
}


bool FKashmirSwordGestureInput::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (Samples.Num() < 2)
    {
        OutReason = TEXT("sword gesture requires at least two samples");
        return false;
    }

    for (const FVector2D& Sample : Samples)
    {
        if (!IsFiniteVector2D(Sample))
        {
            OutReason = TEXT("sword gesture samples must be finite");
            return false;
        }
    }

    if (!FMath::IsFinite(DurationSeconds) || DurationSeconds <= 0.0f)
    {
        OutReason = TEXT("sword gesture duration must be finite and positive");
        return false;
    }

    return true;
}


bool FKashmirSwordActionBinding::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (Family == EKashmirSwordGestureFamily::None)
    {
        OutReason = TEXT("sword action binding requires a gesture family");
        return false;
    }

    if (ActionId.IsNone())
    {
        OutReason = TEXT("sword action binding requires an action id");
        return false;
    }

    return true;
}


bool FKashmirDirectionalSwordConfig::IsValid(FString& OutReason) const
{
    OutReason.Reset();

    if (!FMath::IsFinite(MinimumDragDistance) || MinimumDragDistance < 0.0f)
    {
        OutReason = TEXT("minimum sword drag distance must be finite and non-negative");
        return false;
    }

    if (!FMath::IsFinite(FullIntensityDistance) ||
        FullIntensityDistance <= 0.0f ||
        FullIntensityDistance < MinimumDragDistance)
    {
        OutReason = TEXT("full intensity distance must be finite and at least the minimum drag distance");
        return false;
    }

    if (!FMath::IsFinite(CurvedFamilyThreshold) ||
        CurvedFamilyThreshold < 0.0f ||
        CurvedFamilyThreshold > 1.0f)
    {
        OutReason = TEXT("curved family threshold must be between zero and one");
        return false;
    }

    TSet<uint16> SeenBindings;
    for (const FKashmirSwordActionBinding& Binding : ActionBindings)
    {
        FString BindingReason;
        if (!Binding.IsValid(BindingReason))
        {
            OutReason = FString::Printf(TEXT("invalid sword action binding: %s"), *BindingReason);
            return false;
        }

        const uint16 Key =
            (static_cast<uint16>(Binding.Family) << 8) |
            static_cast<uint16>(Binding.Direction);

        if (SeenBindings.Contains(Key))
        {
            OutReason = TEXT("sword action bindings must not contain duplicate family and direction pairs");
            return false;
        }

        SeenBindings.Add(Key);
    }

    return true;
}


bool FKashmirDirectionalSwordResolver::Resolve(
    const FKashmirSwordGestureInput& Input,
    const FKashmirDirectionalSwordConfig& Config,
    FKashmirDirectionalSwordResult& OutResult,
    FString& OutReason) const
{
    OutResult = {};
    OutReason.Reset();

    if (!Input.IsValid(OutReason) || !Config.IsValid(OutReason))
    {
        return false;
    }

    const FVector2D Displacement = Input.Samples.Last() - Input.Samples[0];
    OutResult.DirectDistance = Displacement.Size();

    for (int32 Index = 1; Index < Input.Samples.Num(); ++Index)
    {
        OutResult.PathLength += (Input.Samples[Index] - Input.Samples[Index - 1]).Size();
    }

    OutResult.AverageSpeed = OutResult.PathLength / Input.DurationSeconds;

    if (OutResult.DirectDistance < Config.MinimumDragDistance)
    {
        OutReason = TEXT("sword gesture is below minimum drag distance");
        OutResult = {};
        return false;
    }

    OutResult.DirectionVector = Displacement.GetSafeNormal();
    OutResult.Direction = QuantizeDirection(OutResult.DirectionVector);
    OutResult.Intensity = FMath::Clamp(
        OutResult.DirectDistance / Config.FullIntensityDistance,
        0.0f,
        1.0f);
    OutResult.Curvature = FMath::Clamp(
        (OutResult.PathLength - OutResult.DirectDistance) / OutResult.DirectDistance,
        0.0f,
        1.0f);
    OutResult.Family =
        OutResult.Curvature >= Config.CurvedFamilyThreshold
            ? EKashmirSwordGestureFamily::Curved
            : EKashmirSwordGestureFamily::Direct;

    const FKashmirSwordActionBinding* Binding =
        Config.ActionBindings.FindByPredicate(
            [&OutResult](const FKashmirSwordActionBinding& Candidate)
            {
                return Candidate.Family == OutResult.Family &&
                    Candidate.Direction == OutResult.Direction;
            });

    if (Binding == nullptr)
    {
        OutReason = TEXT("no sword action binding matches the resolved family and direction");
        OutResult = {};
        return false;
    }

    OutResult.ActionRequest.ActionId = Binding->ActionId;
    OutResult.ActionRequest.Direction = OutResult.DirectionVector;
    OutResult.ActionRequest.Intensity = OutResult.Intensity;
    OutResult.ActionRequest.Curvature = OutResult.Curvature;
    OutResult.bGestureResolved = true;
    return true;
}
