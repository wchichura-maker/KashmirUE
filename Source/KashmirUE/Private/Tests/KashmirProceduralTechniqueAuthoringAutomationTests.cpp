#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirCombatTechnique.h"
#include "Combat/KashmirDirectionalSwordComponent.h"
#include "Combat/KashmirDirectionalSwordProfile.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


namespace
{
    constexpr const TCHAR* StylePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirSword_CombatStyle_Baseline.")
        TEXT("DA_KashmirSword_CombatStyle_Baseline");
    constexpr const TCHAR* ProfilePath =
        TEXT("/Game/KashmirAct/Combat/DirectionalSword/")
        TEXT("DA_KashmirDirectionalSword_Baseline.")
        TEXT("DA_KashmirDirectionalSword_Baseline");
    const FName QuickId(TEXT("Technique.Sword.Diagonal.Rising.Quick"));
    const FName WideId(TEXT("Technique.Sword.Diagonal.Rising.Wide"));

    UKashmirWeaponCombatStyle* LoadStyle()
    {
        return LoadObject<UKashmirWeaponCombatStyle>(nullptr, StylePath);
    }

    UKashmirDirectionalSwordProfile* LoadProfile()
    {
        return LoadObject<UKashmirDirectionalSwordProfile>(nullptr, ProfilePath);
    }

    const FKashmirCombatTechniqueDefinition* FindTechnique(
        const UKashmirWeaponCombatStyle* Style,
        const FName Id)
    {
        return Style != nullptr
            ? Style->Techniques.FindByPredicate(
                [Id](const FKashmirCombatTechniqueDefinition& Candidate)
                {
                    return Candidate.TechniqueId == Id;
                })
            : nullptr;
    }

    bool ProfilesDiffer(
        const FKashmirSwordPoseConfig& A,
        const FKashmirSwordPoseConfig& B)
    {
        return A.LeadHandOffsetAtFullIntensity != B.LeadHandOffsetAtFullIntensity ||
            A.SupportHandOffsetAtFullIntensity != B.SupportHandOffsetAtFullIntensity ||
            !FMath::IsNearlyEqual(A.AimYawAtFullIntensity, B.AimYawAtFullIntensity) ||
            !FMath::IsNearlyEqual(A.MaximumBodyLean, B.MaximumBodyLean);
    }

    const FKashmirCombatTechniqueDefinition* FindBoundTechnique(
        const UKashmirWeaponCombatStyle* Style,
        const int32 BindingIndex)
    {
        return Style != nullptr && Style->SlotBindings.IsValidIndex(BindingIndex)
            ? FindTechnique(Style, Style->SlotBindings[BindingIndex].TechniqueId)
            : nullptr;
    }
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTwoTechniquesOneBaseMotionTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.TwoTechniquesOneBaseMotion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTwoTechniquesOneBaseMotionTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStyle();
    const FKashmirCombatTechniqueDefinition* Quick = FindTechnique(Style, QuickId);
    const FKashmirCombatTechniqueDefinition* Wide = FindTechnique(Style, WideId);
    TestNotNull(TEXT("Quick definition exists"), Quick);
    TestNotNull(TEXT("Wide definition exists"), Wide);
    if (Quick == nullptr || Wide == nullptr)
    {
        return false;
    }
    TestNotEqual(TEXT("Technique identities differ"), Quick->TechniqueId, Wide->TechniqueId);
    TestEqual(TEXT("Base Motion is shared"), Quick->Montage, Wide->Montage);
    TestEqual(TEXT("Kinematic family is shared"), Quick->TechniqueFamily, Wide->TechniqueFamily);
    TestEqual(TEXT("Quick remains DiagonalRising"), Quick->TechniqueFamily, FName(TEXT("DiagonalRising")));
    TestEqual(TEXT("Wide remains DiagonalRising"), Wide->TechniqueFamily, FName(TEXT("DiagonalRising")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirDistinctTechniquePresentationProfilesTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.DistinctPresentationProfiles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirDistinctTechniquePresentationProfilesTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStyle();
    const FKashmirCombatTechniqueDefinition* Quick = FindTechnique(Style, QuickId);
    const FKashmirCombatTechniqueDefinition* Wide = FindTechnique(Style, WideId);
    if (Quick == nullptr || Wide == nullptr)
    {
        AddError(TEXT("Quick and Wide definitions must exist"));
        return false;
    }
    TestTrue(TEXT("Quick owns a procedural presentation override"), Quick->bOverrideSwordPresentation);
    TestTrue(TEXT("Wide owns a procedural presentation override"), Wide->bOverrideSwordPresentation);
    TestTrue(TEXT("Profiles are spatially distinct"),
        ProfilesDiffer(Quick->SwordPresentation, Wide->SwordPresentation));
    FString Reason;
    TestTrue(TEXT("Quick profile validates"), Quick->SwordPresentation.IsValid(Reason));
    Reason.Reset();
    TestTrue(TEXT("Wide profile validates"), Wide->SwordPresentation.IsValid(Reason));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniquePresentationProfileResolutionTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.PresentationProfileResolution",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTechniquePresentationProfileResolutionTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* SourceStyle = LoadStyle();
    UKashmirDirectionalSwordProfile* Profile = LoadProfile();
    const FKashmirCombatTechniqueDefinition* Quick = FindTechnique(SourceStyle, QuickId);
    const FKashmirCombatTechniqueDefinition* Wide = FindTechnique(SourceStyle, WideId);
    if (SourceStyle == nullptr || Profile == nullptr || Quick == nullptr || Wide == nullptr)
    {
        AddError(TEXT("Procedural authoring assets must load"));
        return false;
    }

    const FKashmirCombatTechniqueDefinition* Techniques[] = {Quick, Wide};
    for (const FKashmirCombatTechniqueDefinition* Technique : Techniques)
    {
        UKashmirWeaponCombatStyle* TestStyle = DuplicateObject<UKashmirWeaponCombatStyle>(
            SourceStyle, GetTransientPackage());
        TestStyle->SlotBindings[0].TechniqueId = Technique->TechniqueId;

        UKashmirDirectionalSwordComponent* Sword =
            NewObject<UKashmirDirectionalSwordComponent>();
        Sword->SetProfile(Profile);
        FKashmirTechniqueRequest Request;
        Request.Slot = TestStyle->SlotBindings[0].Slot;
        FString Reason;
        TestTrue(TEXT("Generic TechniqueRequest starts procedural variant"),
            Sword->StartTechniqueRequest(Request, TestStyle, Reason));
        const FKashmirSwordActionPlan Plan = Sword->GetActivePlan();
        TestEqual(TEXT("Technique Base Motion reaches the Sword plan"),
            Plan.Montage, Technique->Montage);
        TestEqual(TEXT("Lead-hand profile reaches the Sword plan"),
            Plan.PoseConfig.LeadHandOffsetAtFullIntensity,
            Technique->SwordPresentation.LeadHandOffsetAtFullIntensity);
        TestEqual(TEXT("Body-lean profile reaches the Sword plan"),
            Plan.PoseConfig.MaximumBodyLean,
            Technique->SwordPresentation.MaximumBodyLean);
    }
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirNoTechniqueSpecificCoreBranchTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.NoTechniqueSpecificCoreBranch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirNoTechniqueSpecificCoreBranchTest::RunTest(const FString& Parameters)
{
    FString Source;
    const FString Path = FPaths::Combine(
        FPaths::ProjectDir(),
        TEXT("Source/KashmirUE/Private/Combat/KashmirDirectionalSwordComponent.cpp"));
    TestTrue(TEXT("Directional Sword core source is readable"),
        FFileHelper::LoadFileToString(Source, *Path));
    TestFalse(TEXT("Core contains no Quick-specific branch"), Source.Contains(TEXT("Rising.Quick")));
    TestFalse(TEXT("Core contains no Wide-specific branch"), Source.Contains(TEXT("Rising.Wide")));
    TestNull(TEXT("No Quick-specific component class exists"),
        FindObject<UClass>(nullptr, TEXT("QuickRisingSwordComponent")));
    TestNull(TEXT("No Wide-specific component class exists"),
        FindObject<UClass>(nullptr, TEXT("WideRisingSwordComponent")));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirProceduralAuthoringExistingBaselineTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.ExistingBaseline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirProceduralAuthoringExistingBaselineTest::RunTest(const FString& Parameters)
{
    const UKashmirWeaponCombatStyle* Style = LoadStyle();
    TestNotNull(TEXT("Sword style loads"), Style);
    if (Style == nullptr)
    {
        return false;
    }
    TestEqual(TEXT("Slots 1-4 remain bound and Slot5 remains unbound"),
        Style->SlotBindings.Num(), 4);
    FString Reason;
    TestTrue(TEXT("Extended style remains valid"), Style->ValidateStyle(Reason));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueRequestRejectsNonCancellableActionTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.NonCancellableRequestRejected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTechniqueRequestRejectsNonCancellableActionTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = LoadStyle();
    UKashmirDirectionalSwordProfile* Profile = LoadProfile();
    if (Style == nullptr || Profile == nullptr || Style->SlotBindings.Num() < 2)
    {
        AddError(TEXT("Baseline style and profile require at least two bound Techniques"));
        return false;
    }

    UKashmirDirectionalSwordComponent* Sword = NewObject<UKashmirDirectionalSwordComponent>();
    Sword->SetProfile(Profile);
    FKashmirTechniqueRequest FirstRequest;
    FirstRequest.Slot = Style->SlotBindings[0].Slot;
    FKashmirTechniqueRequest SecondRequest;
    SecondRequest.Slot = Style->SlotBindings[1].Slot;
    FString Reason;
    TestTrue(TEXT("Initial Technique starts"), Sword->StartTechniqueRequest(FirstRequest, Style, Reason));
    const FKashmirActionRuntimeState BeforeRejectedRequest = Sword->GetRuntimeState();

    Reason.Reset();
    TestFalse(TEXT("Active non-cancellable Technique rejects replacement"),
        Sword->StartTechniqueRequest(SecondRequest, Style, Reason));
    const FKashmirActionRuntimeState AfterRejectedRequest = Sword->GetRuntimeState();
    TestEqual(TEXT("Rejected request preserves active ActionId"),
        AfterRejectedRequest.ActionId, BeforeRejectedRequest.ActionId);
    TestEqual(TEXT("Rejected request preserves phase"),
        AfterRejectedRequest.Phase, BeforeRejectedRequest.Phase);
    TestEqual(TEXT("Rejected request preserves elapsed time"),
        AfterRejectedRequest.Elapsed, BeforeRejectedRequest.Elapsed);
    TestTrue(TEXT("Rejected request remains active"), AfterRejectedRequest.bActive);
    TestTrue(TEXT("Rejection exposes semantic cancellation reason"),
        Reason.Contains(TEXT("ActionCannotBeCancelled")));
    TestEqual(TEXT("Observable rejection reason matches returned reason"),
        Sword->GetLastTechniqueRequestReason(), Reason);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirTechniqueRequestCancelsInAuthoredWindowTest,
    "Kashmir.Combat.ProceduralTechniqueAuthoring.CancellableRequestAccepted",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKashmirTechniqueRequestCancelsInAuthoredWindowTest::RunTest(const FString& Parameters)
{
    UKashmirWeaponCombatStyle* Style = LoadStyle();
    UKashmirDirectionalSwordProfile* SourceProfile = LoadProfile();
    const FKashmirCombatTechniqueDefinition* FirstTechnique = FindBoundTechnique(Style, 0);
    const FKashmirCombatTechniqueDefinition* SecondTechnique = FindBoundTechnique(Style, 1);
    if (Style == nullptr || SourceProfile == nullptr || FirstTechnique == nullptr || SecondTechnique == nullptr)
    {
        AddError(TEXT("Baseline style/profile and first two bound Techniques must load"));
        return false;
    }

    UKashmirDirectionalSwordProfile* TestProfile =
        DuplicateObject<UKashmirDirectionalSwordProfile>(SourceProfile, GetTransientPackage());
    FKashmirSwordAuthoredAction* FirstAction = TestProfile->Actions.FindByPredicate(
        [FirstTechnique](const FKashmirSwordAuthoredAction& Candidate)
        {
            return Candidate.ActionId == FirstTechnique->ActionId;
        });
    if (FirstAction == nullptr)
    {
        AddError(TEXT("First Technique must resolve to an authored Sword action"));
        return false;
    }
    FirstAction->bCancellable = true;
    FirstAction->CancelWindows = {EKashmirActionPhase::Startup};

    UKashmirDirectionalSwordComponent* Sword = NewObject<UKashmirDirectionalSwordComponent>();
    Sword->SetProfile(TestProfile);
    FKashmirTechniqueRequest FirstRequest;
    FirstRequest.Slot = Style->SlotBindings[0].Slot;
    FKashmirTechniqueRequest SecondRequest;
    SecondRequest.Slot = Style->SlotBindings[1].Slot;
    FString Reason;
    TestTrue(TEXT("Cancellable Technique starts"),
        Sword->StartTechniqueRequest(FirstRequest, Style, Reason));
    TestTrue(TEXT("Replacement Technique starts inside authored cancel window"),
        Sword->StartTechniqueRequest(SecondRequest, Style, Reason));
    TestEqual(TEXT("Runtime switches to replacement ActionId"),
        Sword->GetRuntimeState().ActionId, SecondTechnique->ActionId);
    TestTrue(TEXT("Replacement remains active"), Sword->GetRuntimeState().bActive);
    TestTrue(TEXT("Successful request leaves no rejection reason"),
        Sword->GetLastTechniqueRequestReason().IsEmpty());
    return true;
}

#endif
