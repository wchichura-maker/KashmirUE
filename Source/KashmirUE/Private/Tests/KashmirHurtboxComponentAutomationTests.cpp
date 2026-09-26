#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirHurtboxComponent.h"
#include "Combat/KashmirHurtboxProfile.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Engine/SkeletalMesh.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHurtboxRuntimeBuildTest,
    "Kashmir.Combat.Hurtbox.Runtime.BuildHumanoid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)


namespace
{
    FKashmirHurtboxDefinition MakeRuntimeHurtbox(
        const TCHAR* Id,
        const TCHAR* Bone,
        const TCHAR* Region,
        EKashmirHurtboxShape Shape =
            EKashmirHurtboxShape::Capsule)
    {
        FKashmirHurtboxDefinition Result;

        Result.Id =
            FName(Id);

        Result.BoneName =
            FName(Bone);

        Result.HitRegion =
            FGameplayTag::RequestGameplayTag(
                FName(Region)
            );

        Result.Shape =
            Shape;

        Result.Radius =
            8.0f;

        Result.HalfHeight =
            16.0f;

        Result.BoxHalfExtent =
            FVector(
                10.0f,
                12.0f,
                15.0f
            );

        return Result;
    }


    void BuildRuntimeHumanoidProfile(
        UKashmirHurtboxProfile& Profile)
    {
        Profile.Hurtboxes =
        {
            MakeRuntimeHurtbox(
                TEXT("Pelvis"),
                TEXT("pelvis"),
                TEXT("HitRegion.Pelvis"),
                EKashmirHurtboxShape::Box
            ),

            MakeRuntimeHurtbox(
                TEXT("Torso"),
                TEXT("spine_04"),
                TEXT("HitRegion.Torso"),
                EKashmirHurtboxShape::Box
            ),

            MakeRuntimeHurtbox(
                TEXT("Neck"),
                TEXT("neck_01"),
                TEXT("HitRegion.Neck"),
                EKashmirHurtboxShape::Capsule
            ),

            MakeRuntimeHurtbox(
                TEXT("Head"),
                TEXT("head"),
                TEXT("HitRegion.Head"),
                EKashmirHurtboxShape::Sphere
            ),

            MakeRuntimeHurtbox(
                TEXT("Arm.Left"),
                TEXT("upperarm_l"),
                TEXT("HitRegion.Arm.Left")
            ),

            MakeRuntimeHurtbox(
                TEXT("Hand.Left"),
                TEXT("hand_l"),
                TEXT("HitRegion.Hand.Left"),
                EKashmirHurtboxShape::Sphere
            ),

            MakeRuntimeHurtbox(
                TEXT("Arm.Right"),
                TEXT("upperarm_r"),
                TEXT("HitRegion.Arm.Right")
            ),

            MakeRuntimeHurtbox(
                TEXT("Hand.Right"),
                TEXT("hand_r"),
                TEXT("HitRegion.Hand.Right"),
                EKashmirHurtboxShape::Sphere
            ),

            MakeRuntimeHurtbox(
                TEXT("Leg.Left"),
                TEXT("calf_l"),
                TEXT("HitRegion.Leg.Left")
            ),

            MakeRuntimeHurtbox(
                TEXT("Foot.Left"),
                TEXT("foot_l"),
                TEXT("HitRegion.Foot.Left"),
                EKashmirHurtboxShape::Box
            ),

            MakeRuntimeHurtbox(
                TEXT("Leg.Right"),
                TEXT("calf_r"),
                TEXT("HitRegion.Leg.Right")
            ),

            MakeRuntimeHurtbox(
                TEXT("Foot.Right"),
                TEXT("foot_r"),
                TEXT("HitRegion.Foot.Right"),
                EKashmirHurtboxShape::Box
            )
        };
    }
}

struct FHurtboxRuntimeTestWorld
{
    UWorld* World = nullptr;
    AActor* Owner = nullptr;
    USkeletalMeshComponent* Mesh = nullptr;
    UKashmirHurtboxComponent* HurtboxComponent = nullptr;

    bool Initialize(
        USkeletalMesh* SkeletalMesh)
    {
        if (SkeletalMesh == nullptr)
        {
            return false;
        }

        const FName WorldName =
            MakeUniqueObjectName(
                nullptr,
                UWorld::StaticClass(),
                TEXT("KashmirHurtboxRuntimeTestWorld"),
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

        Owner =
            World->SpawnActor<AActor>(
                FVector::ZeroVector,
                FRotator::ZeroRotator
            );

        if (Owner == nullptr)
        {
            return false;
        }

        Mesh =
            NewObject<USkeletalMeshComponent>(
                Owner,
                TEXT("MannyMesh"),
                RF_Transient
            );

        if (Mesh == nullptr)
        {
            return false;
        }

        Owner->SetRootComponent(
            Mesh
        );

        Owner->AddInstanceComponent(
            Mesh
        );

        Mesh->SetSkeletalMesh(
            SkeletalMesh
        );

        Mesh->RegisterComponentWithWorld(
            World
        );

        HurtboxComponent =
            NewObject<UKashmirHurtboxComponent>(
                Owner,
                TEXT("HurtboxComponent"),
                RF_Transient
            );

        if (HurtboxComponent == nullptr)
        {
            return false;
        }

        Owner->AddInstanceComponent(
            HurtboxComponent
        );

        HurtboxComponent->RegisterComponentWithWorld(
            World
        );

        World->Tick(
            LEVELTICK_All,
            0.0f
        );

        return true;
    }

    ~FHurtboxRuntimeTestWorld()
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

bool FKashmirHurtboxRuntimeBuildTest::RunTest(
    const FString& Parameters)
{
    USkeletalMesh* MannyMesh =
        LoadObject<USkeletalMesh>(
            nullptr,
            TEXT(
                "/Game/Characters/Mannequins/Meshes/"
                "SKM_Manny_Simple.SKM_Manny_Simple"
            )
        );

    if (!TestNotNull(
            TEXT("Manny skeletal mesh loads"),
            MannyMesh))
    {
        return false;
    }

    FHurtboxRuntimeTestWorld Fixture;

    if (!TestTrue(
            TEXT(
                "Hurtbox runtime world initializes"
            ),
            Fixture.Initialize(
                MannyMesh
            )))
    {
        return false;
    }

    if (!TestNotNull(
            TEXT("Runtime owner exists"),
            Fixture.Owner))
    {
        return false;
    }

    if (!TestNotNull(
            TEXT("Runtime skeletal mesh exists"),
            Fixture.Mesh))
    {
        return false;
    }

    if (!TestNotNull(
            TEXT("Hurtbox component exists"),
            Fixture.HurtboxComponent))
    {
        return false;
    }

    UKashmirHurtboxProfile* Profile =
        NewObject<UKashmirHurtboxProfile>();

    BuildRuntimeHumanoidProfile(
        *Profile
    );

    FString Reason;

    const bool bBuilt =
        Fixture.HurtboxComponent->Build(
            Fixture.Mesh,
            Profile,
            Reason
        );

    TestTrue(
        TEXT("Humanoid hurtboxes build"),
        bBuilt
    );

    TestTrue(
        TEXT("Runtime build succeeds without error"),
        Reason.IsEmpty()
    );

    TestEqual(
        TEXT("Twelve runtime hurtboxes are created"),
        Fixture.HurtboxComponent->GetHurtboxCount(),
        12
    );

    const TArray<TObjectPtr<UPrimitiveComponent>>& Components =
        Fixture.HurtboxComponent->GetHurtboxComponents();

    int32 ResolvedCount =
        0;

    int32 BoxCount =
        0;

    int32 CapsuleCount =
        0;

    int32 SphereCount =
        0;

    for (int32 Index = 0;
         Index < Components.Num();
         ++Index)
    {
        UPrimitiveComponent* Component =
            Components[Index];

        if (!TestNotNull(
                *FString::Printf(
                    TEXT(
                        "Hurtbox %d has collision component"
                    ),
                    Index
                ),
                Component))
        {
            continue;
        }

        TestEqual(
            *FString::Printf(
                TEXT(
                    "Hurtbox %d is QueryOnly"
                ),
                Index
            ),
            Component->GetCollisionEnabled(),
            ECollisionEnabled::QueryOnly
        );

        TestEqual(
            *FString::Printf(
                TEXT(
                    "Hurtbox %d blocks KashmirMeleeTrace"
                ),
                Index
            ),
            Component->GetCollisionResponseToChannel(
                ECC_GameTraceChannel1
            ),
            ECR_Block
        );

        TestEqual(
            *FString::Printf(
                TEXT(
                    "Hurtbox %d attaches to expected bone"
                ),
                Index
            ),
            Component->GetAttachSocketName(),
            Profile->Hurtboxes[Index].BoneName
        );

        FName HurtboxId;
        FGameplayTag HitRegion;

        const bool bResolved =
            Fixture.HurtboxComponent
                ->ResolveHitComponent(
                    Component,
                    HurtboxId,
                    HitRegion
                );

        TestTrue(
            *FString::Printf(
                TEXT(
                    "Hurtbox %d resolves semantic metadata"
                ),
                Index
            ),
            bResolved
        );

        if (bResolved)
        {
            ++ResolvedCount;

            TestEqual(
                *FString::Printf(
                    TEXT(
                        "Hurtbox %d resolves expected id"
                    ),
                    Index
                ),
                HurtboxId,
                Profile->Hurtboxes[Index].Id
            );

            TestEqual(
                *FString::Printf(
                    TEXT(
                        "Hurtbox %d resolves expected region"
                    ),
                    Index
                ),
                HitRegion,
                Profile->Hurtboxes[Index].HitRegion
            );
        }

        if (Cast<UBoxComponent>(
                Component) != nullptr)
        {
            ++BoxCount;
        }
        else if (
            Cast<UCapsuleComponent>(
                Component) != nullptr)
        {
            ++CapsuleCount;
        }
        else if (
            Cast<USphereComponent>(
                Component) != nullptr)
        {
            ++SphereCount;
        }
    }

    TestEqual(
        TEXT(
            "All twelve components resolve metadata"
        ),
        ResolvedCount,
        12
    );

    TestTrue(
        TEXT("Runtime profile creates box hurtboxes"),
        BoxCount > 0
    );

    TestTrue(
        TEXT("Runtime profile creates capsule hurtboxes"),
        CapsuleCount > 0
    );

    TestTrue(
        TEXT("Runtime profile creates sphere hurtboxes"),
        SphereCount > 0
    );

    Fixture.HurtboxComponent->ClearHurtboxes();

    TestEqual(
        TEXT("Clear removes every hurtbox"),
        Fixture.HurtboxComponent->GetHurtboxCount(),
        0
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHurtboxRuntimeMissingBoneTest,
    "Kashmir.Combat.Hurtbox.Runtime.RejectMissingBone",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHurtboxRuntimeMissingBoneTest::RunTest(
    const FString& Parameters)
{
    USkeletalMesh* MannyMesh =
        LoadObject<USkeletalMesh>(
            nullptr,
            TEXT(
                "/Game/Characters/Mannequins/Meshes/"
                "SKM_Manny_Simple.SKM_Manny_Simple"
            )
        );

    if (MannyMesh == nullptr)
    {
        return false;
    }

    AActor* Owner =
        NewObject<AActor>();

    USkeletalMeshComponent* Mesh =
        NewObject<USkeletalMeshComponent>(
            Owner
        );

    Owner->SetRootComponent(
        Mesh
    );

    Mesh->SetSkeletalMesh(
        MannyMesh
    );

    UKashmirHurtboxComponent* HurtboxComponent =
        NewObject<UKashmirHurtboxComponent>(
            Owner
        );

    UKashmirHurtboxProfile* Profile =
        NewObject<UKashmirHurtboxProfile>();

    Profile->Hurtboxes.Add(
        MakeRuntimeHurtbox(
            TEXT("Invalid"),
            TEXT("bone_that_does_not_exist"),
            TEXT("HitRegion.Head")
        )
    );

    FString Reason;

    const bool bBuilt =
        HurtboxComponent->Build(
            Mesh,
            Profile,
            Reason
        );

    TestFalse(
        TEXT("Missing bone prevents runtime build"),
        bBuilt
    );

    TestEqual(
        TEXT(
            "Failed atomic build creates no hurtboxes"
        ),
        HurtboxComponent->GetHurtboxCount(),
        0
    );

    TestTrue(
        TEXT("Missing bone produces useful reason"),
        Reason.Contains(
            TEXT("missing bone")
        )
    );

    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHurtboxRuntimeWorldSweepTest,
    "Kashmir.Combat.Hurtbox.Runtime.WorldSweepAllRegions",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHurtboxRuntimeWorldSweepTest::RunTest(
    const FString& Parameters)
{
    USkeletalMesh* MannyMesh =
        LoadObject<USkeletalMesh>(
            nullptr,
            TEXT(
                "/Game/Characters/Mannequins/Meshes/"
                "SKM_Manny_Simple.SKM_Manny_Simple"
            )
        );

    if (!TestNotNull(
            TEXT("Manny skeletal mesh loads"),
            MannyMesh))
    {
        return false;
    }

    FHurtboxRuntimeTestWorld Fixture;

    if (!TestTrue(
            TEXT("Hurtbox sweep world initializes"),
            Fixture.Initialize(MannyMesh)))
    {
        return false;
    }

    struct FSweepCase
    {
        const TCHAR* Id;
        const TCHAR* Bone;
        const TCHAR* Region;
        EKashmirHurtboxShape Shape;
    };

    const TArray<FSweepCase> Cases =
    {
        {
            TEXT("Pelvis"),
            TEXT("pelvis"),
            TEXT("HitRegion.Pelvis"),
            EKashmirHurtboxShape::Box
        },
        {
            TEXT("Torso"),
            TEXT("spine_04"),
            TEXT("HitRegion.Torso"),
            EKashmirHurtboxShape::Box
        },
        {
            TEXT("Neck"),
            TEXT("neck_01"),
            TEXT("HitRegion.Neck"),
            EKashmirHurtboxShape::Capsule
        },
        {
            TEXT("Head"),
            TEXT("head"),
            TEXT("HitRegion.Head"),
            EKashmirHurtboxShape::Sphere
        },
        {
            TEXT("Arm.Left"),
            TEXT("upperarm_l"),
            TEXT("HitRegion.Arm.Left"),
            EKashmirHurtboxShape::Capsule
        },
        {
            TEXT("Hand.Left"),
            TEXT("hand_l"),
            TEXT("HitRegion.Hand.Left"),
            EKashmirHurtboxShape::Sphere
        },
        {
            TEXT("Arm.Right"),
            TEXT("upperarm_r"),
            TEXT("HitRegion.Arm.Right"),
            EKashmirHurtboxShape::Capsule
        },
        {
            TEXT("Hand.Right"),
            TEXT("hand_r"),
            TEXT("HitRegion.Hand.Right"),
            EKashmirHurtboxShape::Sphere
        },
        {
            TEXT("Leg.Left"),
            TEXT("calf_l"),
            TEXT("HitRegion.Leg.Left"),
            EKashmirHurtboxShape::Capsule
        },
        {
            TEXT("Foot.Left"),
            TEXT("foot_l"),
            TEXT("HitRegion.Foot.Left"),
            EKashmirHurtboxShape::Box
        },
        {
            TEXT("Leg.Right"),
            TEXT("calf_r"),
            TEXT("HitRegion.Leg.Right"),
            EKashmirHurtboxShape::Capsule
        },
        {
            TEXT("Foot.Right"),
            TEXT("foot_r"),
            TEXT("HitRegion.Foot.Right"),
            EKashmirHurtboxShape::Box
        }
    };

    int32 SuccessfulCases = 0;

    for (const FSweepCase& TestCase : Cases)
    {
        UKashmirHurtboxProfile* Profile =
            NewObject<UKashmirHurtboxProfile>();

        FKashmirHurtboxDefinition Definition =
            MakeRuntimeHurtbox(
                TestCase.Id,
                TestCase.Bone,
                TestCase.Region,
                TestCase.Shape
            );

        /*
         * Give each test shape comfortable
         * dimensions so the sweep crosses it
         * reliably.
         */
        Definition.Radius = 12.0f;
        Definition.HalfHeight = 24.0f;
        Definition.BoxHalfExtent =
            FVector(18.0f, 18.0f, 18.0f);

        Profile->Hurtboxes.Add(
            Definition
        );

        FString Reason;

        const bool bBuilt =
            Fixture.HurtboxComponent->Build(
                Fixture.Mesh,
                Profile,
                Reason
            );

        const FString Prefix =
            FString::Printf(
                TEXT("[%s]"),
                TestCase.Id
            );

        if (!TestTrue(
                *FString::Printf(
                    TEXT("%s builds runtime hurtbox"),
                    *Prefix
                ),
                bBuilt))
        {
            AddError(
                FString::Printf(
                    TEXT("%s build failure: %s"),
                    *Prefix,
                    *Reason
                )
            );

            continue;
        }

        const TArray<TObjectPtr<UPrimitiveComponent>>&
            Components =
                Fixture.HurtboxComponent
                    ->GetHurtboxComponents();

        if (!TestEqual(
                *FString::Printf(
                    TEXT("%s creates exactly one component"),
                    *Prefix
                ),
                Components.Num(),
                1))
        {
            continue;
        }

        UPrimitiveComponent* Hurtbox =
            Components[0];

        if (!TestNotNull(
                *FString::Printf(
                    TEXT("%s component exists"),
                    *Prefix
                ),
                Hurtbox))
        {
            continue;
        }

        Fixture.World->Tick(
            LEVELTICK_All,
            0.0f
        );

        Hurtbox->UpdateComponentToWorld();

        const FVector Center =
            Hurtbox->GetComponentLocation();

        const FVector TraceStart =
            Center +
            FVector(100.0f, 0.0f, 0.0f);

        const FVector TraceEnd =
            Center -
            FVector(100.0f, 0.0f, 0.0f);

        FCollisionQueryParams QueryParams;

        QueryParams.bTraceComplex = false;
        QueryParams.bReturnPhysicalMaterial = false;

        FHitResult Hit;

        const bool bHit =
            Fixture.World->SweepSingleByChannel(
                Hit,
                TraceStart,
                TraceEnd,
                FQuat::Identity,
                ECC_GameTraceChannel1,
                FCollisionShape::MakeSphere(
                    2.0f
                ),
                QueryParams
            );

        if (!TestTrue(
                *FString::Printf(
                    TEXT("%s is physically hittable"),
                    *Prefix
                ),
                bHit))
        {
            Fixture.HurtboxComponent
                ->ClearHurtboxes();

            continue;
        }

        TestEqual(
            *FString::Printf(
                TEXT("%s sweep hits expected actor"),
                *Prefix
            ),
            Hit.GetActor(),
            Fixture.Owner
        );

        TestEqual(
            *FString::Printf(
                TEXT("%s sweep hits dedicated hurtbox component"),
                *Prefix
            ),
            Hit.GetComponent(),
            Hurtbox
        );

        FName ResolvedId;
        FGameplayTag ResolvedRegion;

        const bool bResolved =
            Fixture.HurtboxComponent
                ->ResolveHitComponent(
                    Hit.GetComponent(),
                    ResolvedId,
                    ResolvedRegion
                );

        TestTrue(
            *FString::Printf(
                TEXT("%s resolves hit metadata"),
                *Prefix
            ),
            bResolved
        );

        const FGameplayTag ExpectedRegion =
            FGameplayTag::RequestGameplayTag(
                FName(TestCase.Region)
            );

        const bool bCorrectId =
            ResolvedId ==
            FName(TestCase.Id);

        const bool bCorrectRegion =
            ResolvedRegion ==
            ExpectedRegion;

        TestTrue(
            *FString::Printf(
                TEXT("%s resolves expected hurtbox id"),
                *Prefix
            ),
            bCorrectId
        );

        TestTrue(
            *FString::Printf(
                TEXT("%s resolves expected semantic region"),
                *Prefix
            ),
            bCorrectRegion
        );

        AddInfo(
            FString::Printf(
                TEXT(
                    "%s component='%s' "
                    "id='%s' "
                    "region='%s'"
                ),
                *Prefix,
                *Hurtbox->GetName(),
                *ResolvedId.ToString(),
                *ResolvedRegion.ToString()
            )
        );

        if (
            bHit &&
            bResolved &&
            bCorrectId &&
            bCorrectRegion)
        {
            ++SuccessfulCases;
        }

        Fixture.HurtboxComponent
            ->ClearHurtboxes();
    }

    TestEqual(
        TEXT(
            "All semantic hurtboxes are physically hittable"
        ),
        SuccessfulCases,
        Cases.Num()
    );

    AddInfo(
        FString::Printf(
            TEXT(
                "Dedicated hurtbox sweep coverage: %d/%d"
            ),
            SuccessfulCases,
            Cases.Num()
        )
    );

    return true;
}
#endif