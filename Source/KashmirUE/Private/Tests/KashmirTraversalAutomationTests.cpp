#if WITH_DEV_AUTOMATION_TESTS

#include "KashmirCharacter.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Traversal/KashmirTraversalComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTraversalDetectionTest,
    "Kashmir.Traversal.Detection.Foundation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
    struct FTraversalTestWorld
    {
        UWorld* World = nullptr;
        AKashmirCharacter* Character = nullptr;
        TArray<TObjectPtr<AActor>> Geometry;

        bool Initialize()
        {
            const FName WorldName = MakeUniqueObjectName(
                nullptr,
                UWorld::StaticClass(),
                TEXT("KashmirTraversalTestWorld"),
                EUniqueObjectNameOptions::GloballyUnique);
            FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
            World = UWorld::CreateWorld(
                EWorldType::Game,
                false,
                WorldName,
                GetTransientPackage());
            if (!World)
            {
                return false;
            }
            World->AddToRoot();
            WorldContext.SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
            Character = World->SpawnActor<AKashmirCharacter>(
                FVector(0.0f, 0.0f, 96.0f), FRotator::ZeroRotator);
            return Character != nullptr;
        }

        void ResetGeometry()
        {
            for (AActor* Actor : Geometry)
            {
                if (Actor)
                {
                    Actor->Destroy();
                }
            }
            Geometry.Reset();
            World->Tick(LEVELTICK_All, 0.0f);
        }

        AActor* AddBox(const FVector& Center, const FVector& Extent)
        {
            AActor* Actor = World->SpawnActor<AActor>(Center, FRotator::ZeroRotator);
            UBoxComponent* Box = NewObject<UBoxComponent>(Actor, NAME_None, RF_Transient);
            Actor->SetRootComponent(Box);
            Actor->AddInstanceComponent(Box);
            Box->SetBoxExtent(Extent);
            Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            Box->SetCollisionObjectType(ECC_WorldStatic);
            Box->SetCollisionResponseToAllChannels(ECR_Block);
            Box->RegisterComponentWithWorld(World);
            Actor->SetActorLocation(Center);
            Box->UpdateComponentToWorld();
            Geometry.Add(Actor);
            World->Tick(LEVELTICK_All, 0.0f);
            return Actor;
        }

        AActor* AddObstacle(float Height, float Depth)
        {
            const float FrontX = 85.0f;
            return AddBox(
                FVector(FrontX + Depth * 0.5f, 0.0f, Height * 0.5f),
                FVector(Depth * 0.5f, 80.0f, Height * 0.5f));
        }

        ~FTraversalTestWorld()
        {
            if (World)
            {
                GEngine->DestroyWorldContext(World);
                World->DestroyWorld(false);
                World->RemoveFromRoot();
            }
        }
    };
}

bool FKashmirTraversalDetectionTest::RunTest(const FString& Parameters)
{
    FTraversalTestWorld Fixture;
    if (!TestTrue(TEXT("Traversal test world initializes"), Fixture.Initialize()))
    {
        return false;
    }

    UKashmirTraversalComponent* Component = Fixture.Character->GetTraversalComponent();
    if (!TestNotNull(TEXT("Character owns a traversal component"), Component))
    {
        return false;
    }
    TestFalse(TEXT("Traversal query does not tick"), Component->PrimaryComponentTick.bCanEverTick);

    const FVector InitialLocation = Fixture.Character->GetActorLocation();
    const FVector InitialVelocity = Fixture.Character->GetVelocity();
    const bool InitialWalk = Fixture.Character->IsWalking();
    const bool InitialLock = Fixture.Character->IsLockOnActive();
    const FTraversalQueryResult Empty = Component->QueryTraversal();
    TestFalse(TEXT("No obstacle is invalid"), Empty.bIsValid);
    TestEqual(TEXT("No obstacle type is None"), Empty.Type, ETraversalType::None);
    TestEqual(TEXT("Query preserves transform"), Fixture.Character->GetActorLocation(), InitialLocation);
    TestEqual(TEXT("Query preserves velocity"), Fixture.Character->GetVelocity(), InitialVelocity);
    TestEqual(TEXT("Query preserves gait"), Fixture.Character->IsWalking(), InitialWalk);
    TestEqual(TEXT("Query preserves Lock-On"), Fixture.Character->IsLockOnActive(), InitialLock);

    Fixture.Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    Fixture.Character->RequestTraversalOrJump();
    TestTrue(TEXT("No obstacle preserves native Jump fallback"),
        Fixture.Character->bPressedJump != 0);
    Fixture.Character->StopTraversalOrJump();

    struct FCase
    {
        const TCHAR* Label;
        float Height;
        float Depth;
        ETraversalType Expected;
    };
    const FCase Cases[] = {
        { TEXT("Low"), 50.0f, 50.0f, ETraversalType::VaultLow },
        { TEXT("High"), 95.0f, 60.0f, ETraversalType::VaultHigh },
        { TEXT("Mantle"), 150.0f, 70.0f, ETraversalType::Mantle },
    };

    for (const FCase& TestCase : Cases)
    {
        Fixture.ResetGeometry();
        Fixture.AddObstacle(TestCase.Height, TestCase.Depth);
        const FVector Before = Fixture.Character->GetActorLocation();
        const FVector VelocityBefore = Fixture.Character->GetVelocity();
        const FTraversalQueryResult Result = Component->QueryTraversal();
        TestTrue(FString::Printf(TEXT("%s obstacle is valid"), TestCase.Label), Result.bIsValid);
        TestEqual(FString::Printf(TEXT("%s classification"), TestCase.Label), Result.Type, TestCase.Expected);
        TestTrue(FString::Printf(TEXT("%s height measured"), TestCase.Label),
            FMath::IsNearlyEqual(Result.ObstacleHeight, TestCase.Height, 1.0f));
        TestTrue(FString::Printf(TEXT("%s depth measured"), TestCase.Label),
            FMath::IsNearlyEqual(Result.ObstacleDepth, TestCase.Depth, Component->DepthProbeStep));
        TestTrue(FString::Printf(TEXT("%s destination clearance"), TestCase.Label),
            Result.bHasDestinationClearance);
        TestEqual(FString::Printf(TEXT("%s query preserves transform"), TestCase.Label),
            Fixture.Character->GetActorLocation(), Before);
        TestEqual(FString::Printf(TEXT("%s query preserves velocity"), TestCase.Label),
            Fixture.Character->GetVelocity(), VelocityBefore);
        AddInfo(FString::Printf(
            TEXT("GEOMETRY %s Height=%.1f Depth=%.1f Type=%d Target=(%.1f,%.1f,%.1f) Clearance=%s"),
            TestCase.Label, Result.ObstacleHeight, Result.ObstacleDepth,
            static_cast<uint8>(Result.Type), Result.TargetLocation.X,
            Result.TargetLocation.Y, Result.TargetLocation.Z,
            Result.bHasDestinationClearance ? TEXT("true") : TEXT("false")));
    }

    Fixture.ResetGeometry();
    Fixture.AddObstacle(50.0f, 50.0f);
    Fixture.Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    Fixture.Character->RequestTraversalOrJump();
    TestTrue(TEXT("Valid detection still preserves temporary native Jump fallback"),
        Fixture.Character->bPressedJump != 0);
    Fixture.Character->StopTraversalOrJump();

    Fixture.ResetGeometry();
    Fixture.AddObstacle(220.0f, 50.0f);
    const FTraversalQueryResult TooHigh = Component->QueryTraversal();
    TestFalse(TEXT("Too-high obstacle is rejected"), TooHigh.bIsValid);
    TestEqual(TEXT("Too-high type is None"), TooHigh.Type, ETraversalType::None);

    Fixture.ResetGeometry();
    Fixture.AddObstacle(60.0f, 130.0f);
    const FTraversalQueryResult TooDeep = Component->QueryTraversal();
    TestFalse(TEXT("Excessively deep obstacle is rejected"), TooDeep.bIsValid);

    Fixture.ResetGeometry();
    Fixture.AddObstacle(60.0f, 50.0f);
    // Offset the blocker from the vertical top probes while keeping it inside
    // the destination capsule radius. This isolates the clearance stage.
    Fixture.AddBox(FVector(135.0f, 35.0f, 158.0f), FVector(15.0f, 5.0f, 20.0f));
    const FTraversalQueryResult Blocked = Component->QueryTraversal();
    TestFalse(TEXT("Blocked destination is rejected"), Blocked.bIsValid);
    TestFalse(TEXT("Blocked destination reports no clearance"), Blocked.bHasDestinationClearance);

    TestTrue(TEXT("Upward top surface is accepted"),
        Component->IsTopSurfaceAcceptable(FVector::UpVector));
    TestFalse(TEXT("Vertical/steep surface is rejected as a top"),
        Component->IsTopSurfaceAcceptable(FVector::ForwardVector));
    TestEqual(TEXT("Below-minimum classification is None"),
        Component->ClassifyTraversal(20.0f, 50.0f), ETraversalType::None);
    TestEqual(TEXT("Excess-depth classification is None"),
        Component->ClassifyTraversal(50.0f, 120.0f), ETraversalType::None);

    return true;
}

#endif
