using UnrealBuildTool;

public class KashmirUE : ModuleRules
{
    public KashmirUE(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            "GameplayAbilities", "GameplayTags", "GameplayTasks", "Json"
        });
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AIModule", "JsonUtilities"
        });
    }
}
