// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CradleLightControl : ModuleRules
{
    string SourceCodeDir()
    {
        return PluginDirectory + "/Source/CradleLightControl/";

    }
	public CradleLightControl(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				SourceCodeDir() + "Virtual/Public",
				SourceCodeDir() + "DMX/Public",
				//"../Virtual/Public"
                // ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "DMXProtocol",
                "DMXProtocolEditor",
                "DMXRuntime"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"Slate",
				"SlateCore",
                "InputCore", 
				"Projects",
				"RHI",
				"RenderCore",
				"AppFramework",
				"Json",
				"EditorStyle",
				"DesktopPlatform",
                "DMXProtocol",
                "DMXProtocolEditor",
                "DMXRuntime",
				"PropertyEditor",
				"DetailCustomizations",
				"AssetTools"

				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
