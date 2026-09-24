#include "Traversal/KashmirTraversalComponent.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UKashmirTraversalComponent::UKashmirTraversalComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FTraversalQueryResult UKashmirTraversalComponent::QueryTraversal() const
{
    FTraversalQueryResult Result;
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    const UWorld* World = GetWorld();
    const UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
    if (!Character || !World || !Capsule)
    {
        return Result;
    }

    float CapsuleRadius = 0.0f;
    float CapsuleHalfHeight = 0.0f;
    Capsule->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

    FVector Forward = Character->GetActorForwardVector();
    Forward.Z = 0.0f;
    Forward = Forward.GetSafeNormal();
    if (Forward.IsNearlyZero())
    {
        return Result;
    }

    const float CharacterBaseZ = Character->GetActorLocation().Z - CapsuleHalfHeight;
    const FVector ForwardStart = Character->GetActorLocation()
        + Forward * (CapsuleRadius + CapsuleClearanceMargin)
        + FVector::UpVector * (-CapsuleHalfHeight + MinimumObstacleHeight);
    const FVector ForwardEnd = ForwardStart + Forward * ForwardQueryDistance;
    Result.StartLocation = ForwardStart;

    FHitResult ForwardHit;
    if (!FindForwardObstacle(Forward, CapsuleRadius, CapsuleHalfHeight, ForwardHit))
    {
        DrawQueryDebug(Result, ForwardStart, ForwardEnd);
        return Result;
    }

    FHitResult TopHit;
    if (!FindTopSurface(ForwardHit, Forward, CharacterBaseZ, TopHit)
        || !IsTopSurfaceAcceptable(TopHit.ImpactNormal))
    {
        DrawQueryDebug(Result, ForwardStart, ForwardEnd);
        return Result;
    }

    Result.SurfaceActor = ForwardHit.GetActor();
    Result.SurfaceNormal = ForwardHit.ImpactNormal;
    Result.ObstacleHeight = TopHit.ImpactPoint.Z - CharacterBaseZ;
    if (Result.ObstacleHeight < MinimumObstacleHeight
        || Result.ObstacleHeight > MantleMaximumHeight)
    {
        DrawQueryDebug(Result, ForwardStart, ForwardEnd);
        return Result;
    }

    FVector LastTopPoint = TopHit.ImpactPoint;
    if (!MeasureObstacleDepth(ForwardHit, TopHit, Forward, Result.ObstacleDepth, LastTopPoint))
    {
        DrawQueryDebug(Result, ForwardStart, ForwardEnd);
        return Result;
    }

    Result.Type = ClassifyTraversal(Result.ObstacleHeight, Result.ObstacleDepth);
    if (Result.Type == ETraversalType::None)
    {
        DrawQueryDebug(Result, ForwardStart, ForwardEnd);
        return Result;
    }

    Result.TargetLocation = LastTopPoint
        + FVector::UpVector * (CapsuleHalfHeight + CapsuleClearanceMargin);
    Result.bHasDestinationClearance = HasCapsuleClearance(
        Result.TargetLocation,
        CapsuleRadius,
        CapsuleHalfHeight,
        Result.SurfaceActor);
    Result.bIsValid = Result.bHasDestinationClearance;
    if (!Result.bIsValid)
    {
        Result.Type = ETraversalType::None;
    }

    DrawQueryDebug(Result, ForwardStart, ForwardEnd);
    return Result;
}

ETraversalType UKashmirTraversalComponent::ClassifyTraversal(
    const float ObstacleHeight,
    const float ObstacleDepth) const
{
    if (ObstacleHeight < MinimumObstacleHeight
        || ObstacleHeight > MantleMaximumHeight
        || ObstacleDepth <= 0.0f
        || ObstacleDepth > MaximumTraversableDepth)
    {
        return ETraversalType::None;
    }

    if (ObstacleHeight <= LowVaultMaximumHeight)
    {
        return ETraversalType::VaultLow;
    }
    if (ObstacleHeight <= HighVaultMaximumHeight)
    {
        return ETraversalType::VaultHigh;
    }
    return ETraversalType::Mantle;
}

bool UKashmirTraversalComponent::IsTopSurfaceAcceptable(const FVector& SurfaceNormal) const
{
    return FVector::DotProduct(SurfaceNormal.GetSafeNormal(), FVector::UpVector)
        >= MinimumTopSurfaceUpDot;
}

bool UKashmirTraversalComponent::FindForwardObstacle(
    const FVector& Forward,
    const float CapsuleRadius,
    const float CapsuleHalfHeight,
    FHitResult& OutHit) const
{
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    const UWorld* World = GetWorld();
    if (!Character || !World)
    {
        return false;
    }

    const FVector Start = Character->GetActorLocation()
        + Forward * (CapsuleRadius + CapsuleClearanceMargin)
        + FVector::UpVector * (-CapsuleHalfHeight + MinimumObstacleHeight);
    const FVector End = Start + Forward * ForwardQueryDistance;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(KashmirTraversalForward), false, Character);
    return World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

bool UKashmirTraversalComponent::FindTopSurface(
    const FHitResult& ForwardHit,
    const FVector& Forward,
    const float CharacterBaseZ,
    FHitResult& OutTopHit) const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector ProbePoint = ForwardHit.ImpactPoint + Forward * DepthProbeStep;
    const FVector Start(ProbePoint.X, ProbePoint.Y,
        CharacterBaseZ + MantleMaximumHeight + TopProbeHeight);
    const FVector End(ProbePoint.X, ProbePoint.Y,
        CharacterBaseZ + MinimumObstacleHeight);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(KashmirTraversalTop), false, GetOwner());
    return World->LineTraceSingleByChannel(OutTopHit, Start, End, ECC_Visibility, Params);
}

bool UKashmirTraversalComponent::MeasureObstacleDepth(
    const FHitResult& ForwardHit,
    const FHitResult& TopHit,
    const FVector& Forward,
    float& OutDepth,
    FVector& OutLastTopPoint) const
{
    const UWorld* World = GetWorld();
    if (!World || DepthProbeStep <= 0.0f)
    {
        return false;
    }

    const AActor* SurfaceActor = ForwardHit.GetActor();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(KashmirTraversalDepth), false, GetOwner());
    const float ProbeLimit = MaximumTraversableDepth + DepthProbeStep;
    bool bFoundBackEdge = false;
    OutDepth = 0.0f;

    for (float Distance = DepthProbeStep; Distance <= ProbeLimit; Distance += DepthProbeStep)
    {
        const FVector Probe = ForwardHit.ImpactPoint + Forward * Distance;
        const FVector Start(Probe.X, Probe.Y, TopHit.ImpactPoint.Z + TopProbeHeight);
        const FVector End(Probe.X, Probe.Y, TopHit.ImpactPoint.Z - DepthProbeStep);
        FHitResult SampleHit;
        const bool bHit = World->LineTraceSingleByChannel(
            SampleHit, Start, End, ECC_Visibility, Params);
        const bool bSameTop = bHit
            && SampleHit.GetActor() == SurfaceActor
            && IsTopSurfaceAcceptable(SampleHit.ImpactNormal)
            && FMath::IsNearlyEqual(SampleHit.ImpactPoint.Z, TopHit.ImpactPoint.Z, DepthProbeStep);
        if (!bSameTop)
        {
            bFoundBackEdge = true;
            break;
        }
        OutDepth = Distance;
        OutLastTopPoint = SampleHit.ImpactPoint;
    }

    return bFoundBackEdge && OutDepth > 0.0f && OutDepth <= MaximumTraversableDepth;
}

bool UKashmirTraversalComponent::HasCapsuleClearance(
    const FVector& TargetLocation,
    const float CapsuleRadius,
    const float CapsuleHalfHeight,
    const AActor* SurfaceActor) const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(KashmirTraversalClearance), false, GetOwner());
    // TargetLocation already lifts the capsule by CapsuleClearanceMargin.
    // Test the real character capsule here so the margin is not applied twice.
    const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
        CapsuleRadius,
        CapsuleHalfHeight);
    return !World->OverlapBlockingTestByChannel(
        TargetLocation,
        FQuat::Identity,
        ECC_Pawn,
        CapsuleShape,
        Params);
}

void UKashmirTraversalComponent::DrawQueryDebug(
    const FTraversalQueryResult& Result,
    const FVector& ForwardStart,
    const FVector& ForwardEnd) const
{
#if ENABLE_DRAW_DEBUG
    if (!bDrawDebugTraversal || !GetWorld())
    {
        return;
    }
    const FColor Color = Result.bIsValid ? FColor::Green : FColor::Red;
    DrawDebugLine(GetWorld(), ForwardStart, ForwardEnd, Color, false, 2.0f, 0, 1.5f);
    if (!Result.TargetLocation.IsNearlyZero())
    {
        DrawDebugSphere(GetWorld(), Result.TargetLocation, 8.0f, 12, Color, false, 2.0f);
        if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            float Radius = 0.0f;
            float HalfHeight = 0.0f;
            Character->GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
            DrawDebugCapsule(
                GetWorld(), Result.TargetLocation, HalfHeight, Radius,
                FQuat::Identity, Color, false, 2.0f, 0, 1.0f);
        }
        DrawDebugDirectionalArrow(
            GetWorld(), Result.TargetLocation,
            Result.TargetLocation + Result.SurfaceNormal * 30.0f,
            8.0f, FColor::Cyan, false, 2.0f, 0, 1.5f);
    }
#endif
}
