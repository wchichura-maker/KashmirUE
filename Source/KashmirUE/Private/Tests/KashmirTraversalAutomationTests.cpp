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

    TestFalse(
        TEXT("Traversal query does not tick"),
        Component->PrimaryComponentTick.bCanEverTick
    );

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

    TestTrue(
        TEXT("No obstacle preserves native Jump fallback"),
        Fixture.Character->bPressedJump != 0
    );

    Fixture.Character->StopTraversalOrJump();

    return true;
}


#endif
