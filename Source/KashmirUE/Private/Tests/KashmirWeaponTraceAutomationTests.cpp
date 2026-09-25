#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirWeaponTraceComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirWeaponTraceWorldSweepTest,
    "Kashmir.Combat.WeaponTrace.WorldSweep",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

namespace
{
    struct FWeaponTraceTestWorld
    {
        UWorld* World = nullptr;
        AActor* WeaponActor = nullptr;
        USceneComponent* TraceSource = nullptr;
        UKashmirWeaponTraceComponent* TraceComponent = nullptr;
        AActor* TargetActor = nullptr;

        bool Initialize()
        {
            const FName WorldName =
                MakeUniqueObjectName(
                    nullptr,
                    UWorld::StaticClass(),
                    TEXT("KashmirWeaponTraceTestWorld"),
                    EUniqueObjectNameOptions::GloballyUnique
                );

            FWorldContext& WorldContext =
                GEngine->CreateNewWorldContext(
                    EWorldType::Game
                );

            World =
                UWorld::CreateWorld(
                    EWorldType::Game,
                    false,
                    WorldName,
                    GetTransientPackage()
                );

            if (World == nullptr)
            {
                return false;
            }

            World->AddToRoot();

            WorldContext.SetCurrentWorld(
                World
            );

            World->InitializeActorsForPlay(
                FURL()
            );

            WeaponActor =
                World->SpawnActor<AActor>(
                    FVector::ZeroVector,
                    FRotator::ZeroRotator
                );

            if (WeaponActor == nullptr)
            {
                return false;
            }

            TraceSource =
                NewObject<USceneComponent>(
                    WeaponActor,
                    TEXT("TraceSource"),
                    RF_Transient
                );

            WeaponActor->SetRootComponent(
                TraceSource
            );

            WeaponActor->AddInstanceComponent(
                TraceSource
            );

            TraceSource->RegisterComponentWithWorld(
                World
            );

            TraceComponent =
                NewObject<UKashmirWeaponTraceComponent>(
                    WeaponActor,
                    TEXT("WeaponTrace"),
                    RF_Transient
                );

            WeaponActor->AddInstanceComponent(
                TraceComponent
            );

            TraceComponent->RegisterComponentWithWorld(
                World
            );

            TraceComponent->SetTraceSource(
                TraceSource
            );

            TraceComponent->SetIgnoredActor(
                WeaponActor
            );

            TArray<FName> PointNames;

            PointNames.Add(
                TEXT("Base")
            );

            TraceComponent->SetContactPointNames(
                PointNames
            );

            TargetActor =
                World->SpawnActor<AActor>(
                    FVector(50.0, 0.0, 0.0),
                    FRotator::ZeroRotator
                );

            if (TargetActor == nullptr)
            {
                return false;
            }

            UBoxComponent* TargetBox =
                NewObject<UBoxComponent>(
                    TargetActor,
                    TEXT("TargetBox"),
                    RF_Transient
                );

            TargetActor->SetRootComponent(
                TargetBox
            );

            TargetActor->AddInstanceComponent(
                TargetBox
            );

            TargetBox->SetBoxExtent(
                FVector(10.0, 20.0, 20.0)
            );

            TargetBox->SetCollisionEnabled(
                ECollisionEnabled::QueryOnly
            );

            TargetBox->SetCollisionObjectType(
                ECC_Pawn
            );

            TargetBox->SetCollisionResponseToAllChannels(
                ECR_Ignore
            );

            TargetBox->SetCollisionResponseToChannel(
                ECC_Pawn,
                ECR_Block
            );

            TargetBox->RegisterComponentWithWorld(
                World
            );

            TargetActor->SetActorLocation(
                FVector(50.0, 0.0, 0.0)
            );

            TargetBox->UpdateComponentToWorld();

            World->Tick(
                LEVELTICK_All,
                0.0f
            );

            return true;
        }

        ~FWeaponTraceTestWorld()
        {
            if (World != nullptr)
            {
                GEngine->DestroyWorldContext(
                    World
                );

                World->DestroyWorld(
                    false
                );

                World->RemoveFromRoot();
            }
        }
    };
}

bool FKashmirWeaponTraceWorldSweepTest::RunTest(
    const FString& Parameters)
{
    FWeaponTraceTestWorld Fixture;

    if (!TestTrue(
            TEXT(
                "Weapon trace test world initializes"
            ),
            Fixture.Initialize()))
    {
        return false;
    }

    if (!TestNotNull(
            TEXT("Weapon trace component exists"),
            Fixture.TraceComponent))
    {
        return false;
    }

    TestFalse(
        TEXT("Weapon trace component does not tick"),
        Fixture.TraceComponent
            ->PrimaryComponentTick
            .bCanEverTick
    );

    Fixture.TraceComponent
        ->BeginTraceWindow();

    TArray<FKashmirWeaponTraceHit> Hits;
    FString Reason;

    const bool bFirstSample =
        Fixture.TraceComponent
            ->SampleTrace(
                0.1f,
                Hits,
                Reason
            );

    TestTrue(
        TEXT("First sample initializes trace frame"),
        bFirstSample
    );

    TestEqual(
        TEXT("First sample produces no hit"),
        Hits.Num(),
        0
    );

    Fixture.WeaponActor
        ->SetActorLocation(
            FVector(100.0, 0.0, 0.0)
        );

    Fixture.TraceSource
        ->UpdateComponentToWorld();

    Fixture.World->Tick(
        LEVELTICK_All,
        0.0f
    );

    const bool bSecondSample =
        Fixture.TraceComponent
            ->SampleTrace(
                0.1f,
                Hits,
                Reason
            );

    TestTrue(
        TEXT("Second sample performs sweep"),
        bSecondSample
    );

    TestEqual(
        TEXT("Sweep detects target exactly once"),
        Hits.Num(),
        1
    );

    if (Hits.Num() == 1)
    {
        TestEqual(
            TEXT("Sweep hit expected target"),
            Hits[0].Hit.GetActor(),
            Fixture.TargetActor
        );

        TestEqual(
            TEXT("Contact point id is preserved"),
            Hits[0].ContactPointId,
            FName(TEXT("Base"))
        );

        TestTrue(
            TEXT(
                "Contact speed reflects point movement"
            ),
            FMath::IsNearlyEqual(
                Hits[0].Speed,
                1000.0f,
                0.1f
            )
        );

        TestTrue(
            TEXT(
                "Attack direction follows point movement"
            ),
            Hits[0].AttackDirection.Equals(
                FVector(1.0, 0.0, 0.0),
                0.001
            )
        );

        TestTrue(
            TEXT(
                "Contact velocity is measured from point movement"
            ),
            Hits[0].ContactVelocity.Equals(
                FVector(1000.0, 0.0, 0.0),
                0.1
            )
        );
    }

    TestTrue(
        TEXT("Trace succeeds without error"),
        Reason.IsEmpty()
    );

    Fixture.TraceComponent
        ->EndTraceWindow();

    return true;
}

#endif