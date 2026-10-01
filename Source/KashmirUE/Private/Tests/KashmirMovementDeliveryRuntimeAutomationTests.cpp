#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Animation/AnimInstance.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Combat/KashmirMovementDeliveryComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
    const FName TestActionId(TEXT("Sword.Test.ControlledTranslation"));
    constexpr const TCHAR* MovementDeliveryRuntimeStylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* MovementDeliveryRuntimeProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");

    FKashmirTechniqueMovementSpec MakeControlledSpec()
    {
        FKashmirTechniqueMovementSpec Spec;
        Spec.Delivery = EKashmirMovementDelivery::ControlledTranslation;
        Spec.Distance = 80.0f;
        Spec.Duration = 0.25f;
        Spec.Direction = EKashmirMovementDirection::Forward;
        return Spec;
    }

    FKashmirActionRuntimeState MakeActionState(const bool bActive)
    {
        FKashmirActionRuntimeState State;
        State.bActive = bActive;
        State.ActionId = bActive ? TestActionId : NAME_None;
        return State;
    }

    struct FMovementDeliveryTestWorld
    {
        UWorld* World = nullptr;
        ACharacter* Character = nullptr;
        UKashmirMovementDeliveryComponent* Component = nullptr;

        bool Initialize(const FRotator Rotation = FRotator::ZeroRotator)
        {
            const FName WorldName = MakeUniqueObjectName(
                nullptr,
                UWorld::StaticClass(),
                TEXT("KashmirMovementDeliveryTestWorld"),
                EUniqueObjectNameOptions::GloballyUnique);
            FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
            World = UWorld::CreateWorld(
                EWorldType::Game, false, WorldName, GetTransientPackage());
            if (World == nullptr)
            {
                return false;
            }
            World->AddToRoot();
            Context.SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
            Character = World->SpawnActor<ACharacter>(
                FVector(0.0f, 0.0f, 100.0f), Rotation);
            if (Character == nullptr)
            {
                return false;
            }
            Component = NewObject<UKashmirMovementDeliveryComponent>(
                Character, TEXT("TestMovementDelivery"), RF_Transient);
            Character->AddInstanceComponent(Component);
            Component->RegisterComponentWithWorld(World);
            return true;
        }

        AActor* AddBlockingWall(const float CenterX)
        {
            AActor* Wall = World->SpawnActor<AActor>(
                FVector(CenterX, 0.0f, 100.0f), FRotator::ZeroRotator);
            if (Wall == nullptr) return nullptr;
            UBoxComponent* Box = NewObject<UBoxComponent>(
                Wall, TEXT("MovementDeliveryWall"), RF_Transient);
            Wall->SetRootComponent(Box);
            Wall->AddInstanceComponent(Box);
            Box->SetBoxExtent(FVector(10.0f, 100.0f, 100.0f));
            Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Box->SetCollisionObjectType(ECC_WorldStatic);
            Box->SetCollisionResponseToAllChannels(ECR_Block);
            Box->RegisterComponentWithWorld(World);
            Box->UpdateComponentToWorld();
            World->Tick(LEVELTICK_All, 0.0f);
            return Wall;
        }

        ~FMovementDeliveryTestWorld()
        {
            if (World != nullptr)
            {
                GEngine->DestroyWorldContext(World);
                World->DestroyWorld(false);
                World->RemoveFromRoot();
            }
        }
    };

    struct FMovementDeliveryActionFixture
    {
        FMovementDeliveryTestWorld RuntimeWorld;
        UKashmirDirectionalSwordComponent* Sword = nullptr;
        UKashmirWeaponCombatStyle* Style = nullptr;

        bool Initialize(
            const bool bCancellable,
            const float MovementDuration,
            const float ActionLifetime = 0.60f)
        {
            if (!RuntimeWorld.Initialize()) return false;
            UKashmirWeaponCombatStyle* SourceStyle =
                LoadObject<UKashmirWeaponCombatStyle>(
                    nullptr, MovementDeliveryRuntimeStylePath);
            UKashmirDirectionalSwordProfile* SourceProfile =
                LoadObject<UKashmirDirectionalSwordProfile>(
                    nullptr, MovementDeliveryRuntimeProfilePath);
            if (SourceStyle == nullptr || SourceProfile == nullptr ||
                SourceStyle->SlotBindings.IsEmpty())
            {
                return false;
            }

            Style = DuplicateObject<UKashmirWeaponCombatStyle>(
                SourceStyle, GetTransientPackage());
            UKashmirDirectionalSwordProfile* Profile =
                DuplicateObject<UKashmirDirectionalSwordProfile>(
                    SourceProfile, GetTransientPackage());
            const FName TechniqueId = Style->SlotBindings[0].TechniqueId;
            FKashmirCombatTechniqueDefinition* Technique =
                Style->Techniques.FindByPredicate(
                    [TechniqueId](const FKashmirCombatTechniqueDefinition& Candidate)
                    {
                        return Candidate.TechniqueId == TechniqueId;
                    });
            if (Technique == nullptr) return false;
            FKashmirSwordAuthoredAction* Action = Profile->Actions.FindByPredicate(
                [Technique](const FKashmirSwordAuthoredAction& Candidate)
                {
                    return Candidate.ActionId == Technique->ActionId;
                });
            if (Action == nullptr) return false;

            Technique->MovementSpec = MakeControlledSpec();
            Technique->MovementSpec.Duration = MovementDuration;
            // Deliberately different: this Technique copy is not runtime authority.
            Technique->RuntimeDefinition.StartupDuration = 1.0f;
            Technique->RuntimeDefinition.ActiveDuration = 1.0f;
            Technique->RuntimeDefinition.RecoveryDuration = 1.0f;
            Action->StartupDuration = ActionLifetime / 3.0f;
            Action->ActiveDuration = ActionLifetime / 3.0f;
            Action->RecoveryDuration = ActionLifetime / 3.0f;
            Action->bCancellable = bCancellable;
            Action->CancelWindows = {EKashmirActionPhase::Startup};

            Sword = NewObject<UKashmirDirectionalSwordComponent>(
                RuntimeWorld.Character, TEXT("TestDirectionalSword"), RF_Transient);
            RuntimeWorld.Character->AddInstanceComponent(Sword);
            Sword->RegisterComponentWithWorld(RuntimeWorld.World);
            Sword->SetMovementDeliveryComponent(RuntimeWorld.Component);
            Sword->SetProfile(Profile);
            return true;
        }

        bool Start(FString& OutReason)
        {
            FKashmirTechniqueRequest Request;
            Request.Slot = Style->SlotBindings[0].Slot;
            return Sword->StartTechniqueRequest(Request, Style, OutReason);
        }
    };

    bool ReadProjectSource(const TCHAR* RelativePath, FString& OutSource)
    {
        return FFileHelper::LoadFileToString(
            OutSource,
            *FPaths::Combine(FPaths::ProjectDir(), RelativePath));
    }
}

#define KASHMIR_MOVEMENT_RUNTIME_TEST(ClassName, TestName) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(ClassName, \
        "Kashmir.Combat.MovementDeliveryRuntime." TestName, \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryNoneTest, "NoneDoesNothing")
bool FMovementDeliveryNoneTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    const FVector Before = Fixture.Character->GetActorLocation();
    FString Reason;
    const FKashmirTechniqueMovementSpec Spec;
    TestTrue(TEXT("None starts as a no-op"),
        Fixture.Component->StartDelivery(Spec, TestActionId, Reason));
    TestFalse(TEXT("None never becomes active"), Fixture.Component->IsDeliveryActive());
    TestEqual(TEXT("None does not move the actor"),
        Fixture.Character->GetActorLocation(), Before);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryStartsTest, "ControlledTranslationStarts")
bool FMovementDeliveryStartsTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    FString Reason;
    TestTrue(TEXT("Controlled translation starts"),
        Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason));
    TestTrue(TEXT("Delivery is active"), Fixture.Component->IsDeliveryActive());
    TestEqual(TEXT("Requested distance is observable"),
        Fixture.Component->GetRequestedDistance(), 80.0f);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryYawTest, "ForwardUsesCharacterYaw")
bool FMovementDeliveryYawTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize(FRotator(0.0f, 90.0f, 0.0f)));
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    Fixture.Component->AdvanceDelivery(0.10f, MakeActionState(true), Reason);
    const FVector Location = Fixture.Character->GetActorLocation();
    TestTrue(TEXT("Yaw 90 moves along positive Y"), Location.Y > 31.0f);
    TestTrue(TEXT("Yaw 90 does not use world/camera X"), FMath::Abs(Location.X) < 0.1f);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryRateTest, "DistanceOverDuration")
bool FMovementDeliveryRateTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    TestEqual(TEXT("80 cm over 0.25 s requests 320 cm/s"),
        static_cast<float>(Fixture.Component->GetRequestedVelocity().Size()), 320.0f);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryCompletionTest, "CompletionStopsMovement")
bool FMovementDeliveryCompletionTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    Fixture.Component->AdvanceDelivery(0.25f, MakeActionState(true), Reason);
    const FVector CompletedLocation = Fixture.Character->GetActorLocation();
    TestFalse(TEXT("Delivery completes"), Fixture.Component->IsDeliveryActive());
    TestEqual(TEXT("Completion reason is explicit"),
        Fixture.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Completed);
    Fixture.Component->AdvanceDelivery(0.10f, MakeActionState(true), Reason);
    TestEqual(TEXT("No residual movement after completion"),
        Fixture.Character->GetActorLocation(), CompletedLocation);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryCancelTest, "CancelStopsMovement")
bool FMovementDeliveryCancelTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    Fixture.Component->AdvanceDelivery(0.05f, MakeActionState(true), Reason);
    Fixture.Component->CancelDelivery();
    const FVector CancelledLocation = Fixture.Character->GetActorLocation();
    Fixture.Component->AdvanceDelivery(0.10f, MakeActionState(true), Reason);
    TestEqual(TEXT("Cancel stops movement immediately"),
        Fixture.Character->GetActorLocation(), CancelledLocation);
    TestEqual(TEXT("Cancellation reason is explicit"),
        Fixture.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Cancelled);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryActionEndTest, "ActionCompletionStopsMovement")
bool FMovementDeliveryActionEndTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    const FVector Before = Fixture.Character->GetActorLocation();
    Fixture.Component->AdvanceDelivery(0.10f, MakeActionState(false), Reason);
    TestEqual(TEXT("Ended action produces no further displacement"),
        Fixture.Character->GetActorLocation(), Before);
    TestEqual(TEXT("Action end reason is explicit"),
        Fixture.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::ActionEnded);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(
    FMovementDeliveryAcceptedActionCancelTest,
    "AcceptedActionCancellationStopsDelivery")
bool FMovementDeliveryAcceptedActionCancelTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryActionFixture Fixture;
    TestTrue(TEXT("Cancellable action fixture initializes"), Fixture.Initialize(true, 0.25f));
    FString Reason;
    TestTrue(TEXT("Controlled action starts"), Fixture.Start(Reason));
    TestTrue(TEXT("Action advances before cancellation"),
        Fixture.Sword->AdvanceRuntime(0.05f, Reason));
    const FVector BeforeCancel = Fixture.RuntimeWorld.Character->GetActorLocation();
    TestTrue(TEXT("Action authority accepts cancellation"),
        Fixture.Sword->CancelCurrentAction(Reason));
    const FKashmirActionRuntimeState CancelledState = Fixture.Sword->GetRuntimeState();
    TestFalse(TEXT("ActionRuntime is inactive after accepted cancellation"),
        CancelledState.bActive);
    TestEqual(TEXT("ActionRuntime reports Interrupted"),
        CancelledState.Phase, EKashmirActionPhase::Interrupted);
    TestFalse(TEXT("MovementDelivery is inactive after action cancellation"),
        Fixture.RuntimeWorld.Component->IsDeliveryActive());
    TestEqual(TEXT("MovementDelivery reports Cancelled"),
        Fixture.RuntimeWorld.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Cancelled);
    TestTrue(TEXT("Mid-delivery cancellation moves less than requested"),
        Fixture.RuntimeWorld.Component->GetActualDistance() <
            Fixture.RuntimeWorld.Component->GetRequestedDistance());
    TestTrue(TEXT("Inactive runtime can advance without residual delivery"),
        Fixture.Sword->AdvanceRuntime(0.10f, Reason));
    TestEqual(TEXT("No residual movement follows cancellation"),
        Fixture.RuntimeWorld.Character->GetActorLocation(), BeforeCancel);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(
    FMovementDeliveryRejectedActionCancelTest,
    "RejectedActionCancellationPreservesDelivery")
bool FMovementDeliveryRejectedActionCancelTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryActionFixture Fixture;
    TestTrue(TEXT("Non-cancellable action fixture initializes"), Fixture.Initialize(false, 0.25f));
    FString Reason;
    TestTrue(TEXT("Controlled action starts"), Fixture.Start(Reason));
    TestTrue(TEXT("Action advances before rejected cancellation"),
        Fixture.Sword->AdvanceRuntime(0.05f, Reason));
    TestFalse(TEXT("Action authority rejects cancellation"),
        Fixture.Sword->CancelCurrentAction(Reason));
    TestTrue(TEXT("Rejection reason remains semantic"),
        Reason.Contains(TEXT("ActionCannotBeCancelled")));
    TestTrue(TEXT("ActionRuntime remains active"),
        Fixture.Sword->GetRuntimeState().bActive);
    TestTrue(TEXT("MovementDelivery remains active with ActionRuntime"),
        Fixture.RuntimeWorld.Component->IsDeliveryActive());
    TestEqual(TEXT("Rejected cancellation does not assign a completion reason"),
        Fixture.RuntimeWorld.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::None);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(
    FMovementDeliveryDurationFitsTest,
    "DurationWithinAuthoritativeActionLifetime")
bool FMovementDeliveryDurationFitsTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryActionFixture Fixture;
    TestTrue(TEXT("Equal-duration action fixture initializes"), Fixture.Initialize(false, 0.60f));
    FString Reason;
    TestTrue(TEXT("Delivery fitting authoritative Action lifetime starts"),
        Fixture.Start(Reason));
    TestTrue(TEXT("Equal-duration Action advances to completion"),
        Fixture.Sword->AdvanceRuntime(0.60f, Reason));
    TestEqual(TEXT("Equal-duration delivery reaches requested distance"),
        Fixture.RuntimeWorld.Component->GetActualDistance(), 80.0f, 0.1f);
    TestEqual(TEXT("Equal-duration delivery completes instead of ActionEnded"),
        Fixture.RuntimeWorld.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Completed);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(
    FMovementDeliveryDurationExceedsTest,
    "DurationExceedsAuthoritativeActionLifetimeRejected")
bool FMovementDeliveryDurationExceedsTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryActionFixture Fixture;
    TestTrue(TEXT("Impossible-duration action fixture initializes"), Fixture.Initialize(false, 0.61f));
    FString Reason;
    TestFalse(TEXT("Delivery exceeding authoritative Action lifetime is rejected"),
        Fixture.Start(Reason));
    TestTrue(TEXT("Duration rejection is explicit"),
        Reason.Contains(TEXT("MovementDurationExceedsActionLifetime")));
    TestFalse(TEXT("Rejected plan does not start ActionRuntime"),
        Fixture.Sword->GetRuntimeState().bActive);
    TestFalse(TEXT("Rejected plan does not start MovementDelivery"),
        Fixture.RuntimeWorld.Component->IsDeliveryActive());
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryCollisionTest, "CollisionCanReduceActualDistance")
bool FMovementDeliveryCollisionTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    TestNotNull(TEXT("Blocking wall exists"), Fixture.AddBlockingWall(65.0f));
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    Fixture.Component->AdvanceDelivery(0.25f, MakeActionState(true), Reason);
    TestTrue(TEXT("Collision marks delivery blocked"), Fixture.Component->WasBlocked());
    TestTrue(TEXT("Collision reduces actual distance"),
        Fixture.Component->GetActualDistance() < Fixture.Component->GetRequestedDistance());
    TestEqual(TEXT("Blocked completion reason is explicit"),
        Fixture.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Blocked);
    return true;
}

#if WITH_EDITOR
KASHMIR_MOVEMENT_RUNTIME_TEST(
    FMovementDeliveryTransientBlockerTest,
    "TransientPIEBlockerIsDevelopmentOnly")
bool FMovementDeliveryTransientBlockerTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    const FVector CharacterStart = Fixture.Character->GetActorLocation();
    const FVector RequestedBlockerLocation(100.0f, 0.0f, 100.0f);
    FString Reason;
    AActor* Blocker = Fixture.Component->SpawnTransientDebugBlocker(
        RequestedBlockerLocation,
        FVector(10.0f, 100.0f, 100.0f),
        Reason);
    TestNotNull(TEXT("Development hook creates a transient blocker"), Blocker);
    if (Blocker == nullptr) return false;
    TestTrue(TEXT("Debug blocker cannot be persisted"),
        Blocker->HasAnyFlags(RF_Transient));
    const FVector ActualBlockerLocation = Blocker->GetActorLocation();
    TestTrue(TEXT("Requested blocker location is applied to the root"),
        FVector::Dist(RequestedBlockerLocation, ActualBlockerLocation) <= 0.1f);
    TestTrue(TEXT("Character starts in free space before the blocker"),
        FVector::Dist(CharacterStart, ActualBlockerLocation) > 50.0f);
    UPrimitiveComponent* Primitive =
        Cast<UPrimitiveComponent>(Blocker->GetRootComponent());
    TestNotNull(TEXT("Debug blocker owns collision geometry"), Primitive);
    if (Primitive != nullptr)
    {
        TestEqual(TEXT("Debug blocker blocks Pawn movement"),
            Primitive->GetCollisionResponseToChannel(ECC_Pawn), ECR_Block);
    }
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    Fixture.Component->AdvanceDelivery(0.25f, MakeActionState(true), Reason);
    AddInfo(FString::Printf(
        TEXT("blocker requested=(%.3f,%.3f,%.3f) actual=(%.3f,%.3f,%.3f) requested_distance=%.3f actual_distance=%.3f"),
        RequestedBlockerLocation.X, RequestedBlockerLocation.Y,
        RequestedBlockerLocation.Z, ActualBlockerLocation.X,
        ActualBlockerLocation.Y, ActualBlockerLocation.Z,
        Fixture.Component->GetRequestedDistance(),
        Fixture.Component->GetActualDistance()));
    TestTrue(TEXT("Blocker reduces delivered distance"),
        Fixture.Component->GetRequestedDistance() >
            Fixture.Component->GetActualDistance());
    TestTrue(TEXT("MovementDelivery records a blocking hit"),
        Fixture.Component->WasBlocked());
    TestEqual(TEXT("Transient blocker exercises Blocked completion"),
        Fixture.Component->GetCompletionReason(),
        EKashmirMovementDeliveryCompletionReason::Blocked);
    return true;
}
#endif

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryIntentTest, "MovementIntentIndependent")
bool FMovementDeliveryIntentTest::RunTest(const FString& Parameters)
{
    FKashmirSwordActionPlan StationaryPlan;
    StationaryPlan.MovementIntent = EKashmirMovementIntent::Stationary;
    StationaryPlan.MovementSpec = MakeControlledSpec();
    FKashmirSwordActionPlan FullBodyPlan = StationaryPlan;
    FullBodyPlan.MovementIntent = EKashmirMovementIntent::FullBody;
    TestEqual(TEXT("Stationary keeps controlled delivery"),
        StationaryPlan.MovementSpec.Delivery,
        EKashmirMovementDelivery::ControlledTranslation);
    TestEqual(TEXT("FullBody keeps the same controlled delivery"),
        FullBodyPlan.MovementSpec.Delivery,
        EKashmirMovementDelivery::ControlledTranslation);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryRootMotionTest, "RootMotionIndependent")
bool FMovementDeliveryRootMotionTest::RunTest(const FString& Parameters)
{
    FMovementDeliveryTestWorld Fixture;
    TestTrue(TEXT("World initializes"), Fixture.Initialize());
    UAnimInstance* AnimInstance = NewObject<UAnimInstance>(Fixture.Character->GetMesh());
    AnimInstance->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
    FString Reason;
    Fixture.Component->StartDelivery(MakeControlledSpec(), TestActionId, Reason);
    Fixture.Component->AdvanceDelivery(0.10f, MakeActionState(true), Reason);
    TestEqual(TEXT("Movement delivery does not alter RootMotionMode"),
        AnimInstance->RootMotionMode.GetValue(),
        ERootMotionMode::RootMotionFromMontagesOnly);
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryDamageTest, "DamageUnchanged")
bool FMovementDeliveryDamageTest::RunTest(const FString& Parameters)
{
    FString Source;
    TestTrue(TEXT("Damage resolver source is readable"), ReadProjectSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirDamageResolver.cpp"), Source));
    TestFalse(TEXT("Damage resolver does not consume movement delivery"),
        Source.Contains(TEXT("MovementDelivery")) || Source.Contains(TEXT("MovementSpec")));
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryTraceTest, "WeaponTraceUnchanged")
bool FMovementDeliveryTraceTest::RunTest(const FString& Parameters)
{
    FString Source;
    TestTrue(TEXT("Weapon trace source is readable"), ReadProjectSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirWeaponTraceComponent.cpp"), Source));
    TestFalse(TEXT("Weapon trace does not consume movement delivery"),
        Source.Contains(TEXT("MovementDelivery")) || Source.Contains(TEXT("MovementSpec")));
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryEvidenceTest, "HitEvidenceUnchanged")
bool FMovementDeliveryEvidenceTest::RunTest(const FString& Parameters)
{
    FString Source;
    TestTrue(TEXT("Hit evidence source is readable"), ReadProjectSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirHitEvidenceBuilder.cpp"), Source));
    TestFalse(TEXT("Hit evidence does not consume movement delivery"),
        Source.Contains(TEXT("MovementDelivery")) || Source.Contains(TEXT("MovementSpec")));
    return true;
}

KASHMIR_MOVEMENT_RUNTIME_TEST(FMovementDeliveryNoTeleportTest, "NoDirectActorTeleport")
bool FMovementDeliveryNoTeleportTest::RunTest(const FString& Parameters)
{
    FString Source;
    TestTrue(TEXT("Movement delivery source is readable"), ReadProjectSource(
        TEXT("Source/KashmirUE/Private/Combat/KashmirMovementDeliveryComponent.cpp"), Source));
    TestFalse(TEXT("Executor never uses SetActorLocation"),
        Source.Contains(TEXT("SetActorLocation")));
    TestTrue(TEXT("Executor moves through CharacterMovement sweep"),
        Source.Contains(TEXT("SafeMoveUpdatedComponent")));
    return true;
}

#undef KASHMIR_MOVEMENT_RUNTIME_TEST

#endif
