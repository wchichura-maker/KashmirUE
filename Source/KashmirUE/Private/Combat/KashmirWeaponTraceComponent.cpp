#include "Combat/KashmirWeaponTraceComponent.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UKashmirWeaponTraceComponent::UKashmirWeaponTraceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UKashmirWeaponTraceComponent::SetTraceSource(
    USceneComponent* InTraceSource)
{
    TraceSource = InTraceSource;
}

void UKashmirWeaponTraceComponent::SetContactPointNames(
    const TArray<FName>& InPointNames)
{
    ContactPointNames = InPointNames;
}

void UKashmirWeaponTraceComponent::SetIgnoredActor(
    AActor* InIgnoredActor)
{
    IgnoredActor = InIgnoredActor;
}

void UKashmirWeaponTraceComponent::BeginTraceWindow()
{
    bTraceWindowActive = true;
    bHasPreviousFrame = false;

    PreviousFrame =
        FKashmirWeaponContactFrame();

    HitActorsThisWindow.Reset();
}

void UKashmirWeaponTraceComponent::EndTraceWindow()
{
    bTraceWindowActive = false;
    bHasPreviousFrame = false;

    PreviousFrame =
        FKashmirWeaponContactFrame();

    HitActorsThisWindow.Reset();
}

bool UKashmirWeaponTraceComponent::BuildCurrentFrame(
    FKashmirWeaponContactFrame& OutFrame,
    FString& OutReason) const
{
    OutFrame =
        FKashmirWeaponContactFrame();

    if (TraceSource == nullptr)
    {
        OutReason =
            TEXT("weapon trace source is required");

        return false;
    }

    if (ContactPointNames.IsEmpty())
    {
        OutReason =
            TEXT("weapon trace requires contact points");

        return false;
    }

    for (const FName PointName :
        ContactPointNames)
    {
        if (PointName.IsNone())
        {
            OutReason =
                TEXT("weapon contact point name is required");

            return false;
        }

        const bool bHasSocket =
            TraceSource->DoesSocketExist(
                PointName
            );

        // "Base" may fall back to the component origin.
        // This exists for the generic test fixture.
        if (!bHasSocket &&
            PointName != TEXT("Base"))
        {
            OutReason =
                FString::Printf(
                    TEXT(
                        "weapon contact point does not exist: %s"
                    ),
                    *PointName.ToString()
                );

            return false;
        }

        FKashmirWeaponContactPoint Point;

        Point.Id =
            PointName;

        if (bHasSocket)
        {
            Point.Position =
                TraceSource
                    ->GetSocketTransform(
                        PointName,
                        RTS_World
                    )
                    .GetLocation();
        }
        else
        {
            Point.Position =
                TraceSource
                    ->GetComponentLocation();
        }

        OutFrame.Points.Add(
            Point
        );
    }

    return OutFrame.IsValid(
        OutReason
    );
}

bool UKashmirWeaponTraceComponent::SampleTrace(
    const float DeltaSeconds,
    TArray<FKashmirWeaponTraceHit>& OutHits,
    FString& OutReason)
{
    OutHits.Reset();
    OutReason.Reset();

    if (!bTraceWindowActive)
    {
        OutReason =
            TEXT("weapon trace window is not active");

        return false;
    }

    if (!FMath::IsFinite(DeltaSeconds) ||
        DeltaSeconds <= 0.0f)
    {
        OutReason =
            TEXT(
                "weapon trace requires positive delta seconds"
            );

        return false;
    }

    UWorld* World =
        GetWorld();

    if (World == nullptr)
    {
        OutReason =
            TEXT("weapon trace requires world");

        return false;
    }

    FKashmirWeaponContactFrame CurrentFrame;

    if (!BuildCurrentFrame(
            CurrentFrame,
            OutReason))
    {
        return false;
    }

    // The first sample establishes the initial pose.
    if (!bHasPreviousFrame)
    {
        PreviousFrame =
            CurrentFrame;

        bHasPreviousFrame = true;

        return true;
    }

    TArray<FKashmirWeaponSweepSegment> Segments;

    if (!FKashmirWeaponContactBuilder::BuildSweepSegments(
            PreviousFrame,
            CurrentFrame,
            Segments,
            OutReason))
    {
        return false;
    }

    FCollisionQueryParams QueryParams(
        SCENE_QUERY_STAT(KashmirWeaponTrace),
        false
    );

    if (GetOwner() != nullptr)
    {
        QueryParams.AddIgnoredActor(
            GetOwner()
        );
    }

    if (IgnoredActor != nullptr)
    {
        QueryParams.AddIgnoredActor(
            IgnoredActor
        );
    }

    const FCollisionShape TraceShape =
        FCollisionShape::MakeSphere(
            FMath::Max(
                0.1f,
                TraceRadius
            )
        );

    for (const FKashmirWeaponSweepSegment& Segment :
        Segments)
    {
        if (Segment.Distance <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        const FVector ContactVelocity =
            Segment.Delta / DeltaSeconds;

        const float ContactSpeed =
            ContactVelocity.Size();

        const FVector AttackDirection =
            ContactVelocity.GetSafeNormal();

        TArray<FHitResult> SegmentHits;

        World->SweepMultiByChannel(
            SegmentHits,
            Segment.PreviousPosition,
            Segment.CurrentPosition,
            FQuat::Identity,
            TraceChannel,
            TraceShape,
            QueryParams
        );

        for (const FHitResult& Hit :
            SegmentHits)
        {
            AActor* HitActor =
                Hit.GetActor();

            if (HitActor == nullptr)
            {
                continue;
            }

            if (HitActorsThisWindow.Contains(
                    HitActor))
            {
                continue;
            }

            HitActorsThisWindow.Add(
                HitActor
            );

            FKashmirWeaponTraceHit TraceHit;

            TraceHit.Hit =
                Hit;

            TraceHit.ContactPointId =
                Segment.PointId;

            TraceHit.ContactVelocity =
                ContactVelocity;

            TraceHit.AttackDirection =
                AttackDirection;

            TraceHit.Speed =
                ContactSpeed;

            OutHits.Add(
                TraceHit
            );
        }
    }

    PreviousFrame =
        CurrentFrame;

    return true;
}