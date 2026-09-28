#include "Contracts/KashmirActionContracts.h"

bool FKashmirActionRequest::IsValid(FString& OutReason) const
{
    if (ActionId.IsNone())
    {
        OutReason = TEXT("action_id cannot be empty");
        return false;
    }
    if (!FMath::IsFinite(Intensity) || Intensity < 0.0f)
    {
        OutReason = TEXT("intensity cannot be negative");
        return false;
    }
    if (!FMath::IsFinite(Curvature) || Curvature < 0.0f || Curvature > 1.0f)
    {
        OutReason = TEXT("curvature must be between zero and one");
        return false;
    }
    OutReason.Reset();
    return true;
}
