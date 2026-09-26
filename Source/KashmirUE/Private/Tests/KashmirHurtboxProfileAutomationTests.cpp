#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/KashmirHurtboxProfile.h"
#include "Misc/AutomationTest.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHurtboxProfileValidTest,
    "Kashmir.Combat.Hurtbox.Profile.ValidHumanoid",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)


namespace
{
    FKashmirHurtboxDefinition MakeHurtbox(
        const TCHAR* Id,
        const TCHAR* Bone,
        const TCHAR* Region)
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
            EKashmirHurtboxShape::Capsule;

        Result.Radius =
            10.0f;

        Result.HalfHeight =
            20.0f;

        return Result;
    }

    void BuildHumanoidProfile(
        UKashmirHurtboxProfile& Profile)
    {
        Profile.Hurtboxes =
        {
            MakeHurtbox(
                TEXT("Pelvis"),
                TEXT("pelvis"),
                TEXT("HitRegion.Pelvis")
            ),

            MakeHurtbox(
                TEXT("Torso"),
                TEXT("spine_04"),
                TEXT("HitRegion.Torso")
            ),

            MakeHurtbox(
                TEXT("Neck"),
                TEXT("neck_01"),
                TEXT("HitRegion.Neck")
            ),

            MakeHurtbox(
                TEXT("Head"),
                TEXT("head"),
                TEXT("HitRegion.Head")
            ),

            MakeHurtbox(
                TEXT("Arm.Left"),
                TEXT("upperarm_l"),
                TEXT("HitRegion.Arm.Left")
            ),

            MakeHurtbox(
                TEXT("Hand.Left"),
                TEXT("hand_l"),
                TEXT("HitRegion.Hand.Left")
            ),

            MakeHurtbox(
                TEXT("Arm.Right"),
                TEXT("upperarm_r"),
                TEXT("HitRegion.Arm.Right")
            ),

            MakeHurtbox(
                TEXT("Hand.Right"),
                TEXT("hand_r"),
                TEXT("HitRegion.Hand.Right")
            ),

            MakeHurtbox(
                TEXT("Leg.Left"),
                TEXT("calf_l"),
                TEXT("HitRegion.Leg.Left")
            ),

            MakeHurtbox(
                TEXT("Foot.Left"),
                TEXT("foot_l"),
                TEXT("HitRegion.Foot.Left")
            ),

            MakeHurtbox(
                TEXT("Leg.Right"),
                TEXT("calf_r"),
                TEXT("HitRegion.Leg.Right")
            ),

            MakeHurtbox(
                TEXT("Foot.Right"),
                TEXT("foot_r"),
                TEXT("HitRegion.Foot.Right")
            )
        };
    }
}


bool FKashmirHurtboxProfileValidTest::RunTest(
    const FString& Parameters)
{
    UKashmirHurtboxProfile* Profile =
        NewObject<UKashmirHurtboxProfile>();

    if (!TestNotNull(
            TEXT("Hurtbox profile exists"),
            Profile))
    {
        return false;
    }

    BuildHumanoidProfile(
        *Profile
    );

    FString Reason;

    const bool bValid =
        Profile->ValidateProfile(
            Reason
        );

    TestTrue(
        TEXT(
            "Humanoid hurtbox profile validates"
        ),
        bValid
    );

    TestEqual(
        TEXT(
            "Humanoid profile contains twelve semantic hurtboxes"
        ),
        Profile->Hurtboxes.Num(),
        12
    );

    TestTrue(
        TEXT(
            "Profile validation succeeds without error"
        ),
        Reason.IsEmpty()
    );

    static const TCHAR* ExpectedIds[] =
    {
        TEXT("Pelvis"),
        TEXT("Torso"),
        TEXT("Neck"),
        TEXT("Head"),

        TEXT("Arm.Left"),
        TEXT("Hand.Left"),

        TEXT("Arm.Right"),
        TEXT("Hand.Right"),

        TEXT("Leg.Left"),
        TEXT("Foot.Left"),

        TEXT("Leg.Right"),
        TEXT("Foot.Right")
    };

    for (const TCHAR* ExpectedId :
        ExpectedIds)
    {
        const FKashmirHurtboxDefinition* Found =
            Profile->FindById(
                FName(ExpectedId)
            );

        TestNotNull(
            *FString::Printf(
                TEXT(
                    "Profile contains hurtbox '%s'"
                ),
                ExpectedId
            ),
            Found
        );
    }

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHurtboxProfileDuplicateTest,
    "Kashmir.Combat.Hurtbox.Profile.RejectDuplicateId",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHurtboxProfileDuplicateTest::RunTest(
    const FString& Parameters)
{
    UKashmirHurtboxProfile* Profile =
        NewObject<UKashmirHurtboxProfile>();

    Profile->Hurtboxes.Add(
        MakeHurtbox(
            TEXT("Head"),
            TEXT("head"),
            TEXT("HitRegion.Head")
        )
    );

    Profile->Hurtboxes.Add(
        MakeHurtbox(
            TEXT("Head"),
            TEXT("neck_01"),
            TEXT("HitRegion.Neck")
        )
    );

    FString Reason;

    const bool bValid =
        Profile->ValidateProfile(
            Reason
        );

    TestFalse(
        TEXT(
            "Duplicate hurtbox id is rejected"
        ),
        bValid
    );

    TestTrue(
        TEXT(
            "Duplicate rejection supplies reason"
        ),
        Reason.Contains(
            TEXT("duplicate hurtbox id")
        )
    );

    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FKashmirHurtboxProfileInvalidShapeTest,
    "Kashmir.Combat.Hurtbox.Profile.RejectInvalidShape",
    EAutomationTestFlags::EditorContext |
    EAutomationTestFlags::EngineFilter
)

bool FKashmirHurtboxProfileInvalidShapeTest::RunTest(
    const FString& Parameters)
{
    UKashmirHurtboxProfile* Profile =
        NewObject<UKashmirHurtboxProfile>();

    FKashmirHurtboxDefinition Hurtbox =
        MakeHurtbox(
            TEXT("Head"),
            TEXT("head"),
            TEXT("HitRegion.Head")
        );

    Hurtbox.Radius =
        -1.0f;

    Profile->Hurtboxes.Add(
        Hurtbox
    );

    FString Reason;

    const bool bValid =
        Profile->ValidateProfile(
            Reason
        );

    TestFalse(
        TEXT(
            "Invalid hurtbox dimensions are rejected"
        ),
        bValid
    );

    return true;
}

#endif