
using UnrealBuildTool;

public class PhysicalObjectTracker : ModuleRules
{
    public PhysicalObjectTracker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
        );
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"SteamVR", 
				"Engine",
				"HeadMountedDisplay"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"SteamVRInputDevice"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
