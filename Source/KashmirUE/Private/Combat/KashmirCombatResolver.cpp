#include "Combat/KashmirCombatResolver.h"

bool FKashmirCombatResolver::Resolve(
    const FKashmirCombatActionDefinition& Definition,
    const FKashmirCombatResolveContext& Context,
    FKashmirCombatResult& OutResult,
    FString& OutReason
) const
{
    OutResult = {};
    OutReason.Reset();

    if (!ResolveDelivery(
        Definition,
        Context,
        OutResult,
        OutReason))
    {
        return false;
    }

    if (!ResolveTargets(
        Definition,
        Context,
        OutResult,
        OutReason))
    {
        return false;
    }

    ResolveEffects(
        Definition,
        OutResult
    );

    return true;
}

bool FKashmirCombatResolver::ResolveDelivery(
    const FKashmirCombatActionDefinition& Definition,
    const FKashmirCombatResolveContext& Context,
    FKashmirCombatResult& OutResult,
    FString& OutReason
) const
{
    switch (Definition.Delivery)
    {
        case EKashmirDeliveryType::Self:
        case EKashmirDeliveryType::Contact:
        case EKashmirDeliveryType::Target:
        {
            OutResult.bDelivered = true;
            return true;
        }

        default:
        {
            OutReason =
                TEXT("delivery type is not implemented yet");

            return false;
        }
    }
}

bool FKashmirCombatResolver::ResolveTargets(
    const FKashmirCombatActionDefinition& Definition,
    const FKashmirCombatResolveContext& Context,
    FKashmirCombatResult& OutResult,
    FString& OutReason
) const
{
    switch (Definition.Target)
    {
        case EKashmirTargetType::Self:
        {
            if (Context.InstigatorId.IsNone())
            {
                OutReason =
                    TEXT("self target requires instigator id");

                return false;
            }

            OutResult.TargetIds.Add(
                Context.InstigatorId
            );

            return true;
        }

        case EKashmirTargetType::Enemy:
        case EKashmirTargetType::Entity:
        {
            if (Context.CandidateTargetIds.IsEmpty())
            {
                OutReason =
                    TEXT("target resolution requires at least one candidate");

                return false;
            }

            OutResult.TargetIds =
                Context.CandidateTargetIds;

            return true;
        }

        default:
        {
            OutReason =
                TEXT("target type is not implemented yet");

            return false;
        }
    }
}

void FKashmirCombatResolver::ResolveEffects(
    const FKashmirCombatActionDefinition& Definition,
    FKashmirCombatResult& OutResult) const
{
    for (const FName TargetId :
        OutResult.TargetIds)
    {
        for (const EKashmirResolutionType Resolution :
            Definition.Effects)
        {
            const FGameplayTag EffectTag =
                ResolveEffectTag(
                    Resolution
                );

            FKashmirEffectResult EffectResult;

            EffectResult.Resolution =
                Resolution;

            EffectResult.TargetId =
                TargetId;

            EffectResult.EffectTag =
                EffectTag;

            OutResult.Effects.Add(
                EffectResult
            );

            if (EffectTag.IsValid())
            {
                OutResult.EffectTags.AddTag(
                    EffectTag
                );
            }
        }
    }
}
FGameplayTag FKashmirCombatResolver::ResolveEffectTag(
    const EKashmirResolutionType Resolution) const
{
    const TCHAR* TagName = nullptr;

    switch (Resolution)
    {
        case EKashmirResolutionType::Damage:
            TagName = TEXT("Effect.Damage");
            break;

        case EKashmirResolutionType::Heal:
            TagName = TEXT("Effect.Heal");
            break;

        case EKashmirResolutionType::Control:
            TagName = TEXT("Effect.Control");
            break;

        case EKashmirResolutionType::Modify:
            TagName = TEXT("Effect.Modify");
            break;

        case EKashmirResolutionType::Displace:
            TagName = TEXT("Effect.Displace");
            break;

        case EKashmirResolutionType::Create:
            TagName = TEXT("Effect.Create");
            break;

        case EKashmirResolutionType::Destroy:
            TagName = TEXT("Effect.Destroy");
            break;

        case EKashmirResolutionType::Information:
            TagName = TEXT("Effect.Information");
            break;

        default:
            return FGameplayTag();
    }

    return FGameplayTag::RequestGameplayTag(
        FName(TagName)
    );
}