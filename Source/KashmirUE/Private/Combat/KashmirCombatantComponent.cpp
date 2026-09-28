#include "Combat/KashmirCombatantComponent.h"

#include "GameFramework/Actor.h"


UKashmirCombatantComponent::UKashmirCombatantComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    BlockState.HalfAngleDegrees = 60.0f;
    ParryState.Window.StartTimeSeconds = 0.05f;
    ParryState.Window.EndTimeSeconds = 0.20f;
}


void UKashmirCombatantComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (ParryState.bActive && FMath::IsFinite(DeltaTime) && DeltaTime > 0.0f)
    {
        ParryState.ElapsedTimeSeconds += DeltaTime;
    }
}


void UKashmirCombatantComponent::BeginParry()
{
    ParryState.bActive = true;
    ParryState.ElapsedTimeSeconds = 0.0f;
}


FKashmirDefensePipelineInput
UKashmirCombatantComponent::BuildDefenseInput() const
{
    FKashmirDefensePipelineInput Result;
    Result.BlockState = BlockState;
    if (const AActor* Owner = GetOwner())
    {
        Result.BlockState.ForwardDirection = Owner->GetActorForwardVector();
    }
    Result.ParryState = ParryState;
    Result.DeflectStrengthMultiplier = DeflectStrengthMultiplier;
    Result.GuardDamageMultiplier = GuardDamageMultiplier;
    Result.AvailableStamina = Stamina;
    Result.StaggerConfig = StaggerConfig;
    Result.PhysicalReactionConfig = PhysicalReactionConfig;
    return Result;
}


bool UKashmirCombatantComponent::ApplyResolvedMelee(
    const FKashmirDirectionalMeleeResult& Result,
    FKashmirCombatantApplicationResult& OutApplication,
    FString& OutReason)
{
    OutApplication = {};
    OutReason.Reset();
    if (!Result.bResolved)
    {
        OutReason = TEXT("combatant requires a resolved directional melee result");
        return false;
    }

    OutApplication.StaminaBefore = Stamina;
    if (Result.Defense.DefenseResult.Guard.bGuardProcessed)
    {
        Stamina = Result.Defense.DefenseResult.Guard.StaminaAfter;
    }
    OutApplication.StaminaAfter = Stamina;

    FKashmirEffectApplier Applier;
    for (const FKashmirEffectResult& Effect :
        Result.Defense.HitResult.CombatResult.Effects)
    {
        FKashmirEffectApplicationResult Application;
        if (!Applier.Apply(Effect, Health, Application, OutReason))
        {
            OutApplication = {};
            return false;
        }
        OutApplication.Effects.Add(Application);
        OutApplication.bApplied |= Application.bApplied;
    }
    return true;
}
