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
    OutReason.Reset();
    return true;
}
