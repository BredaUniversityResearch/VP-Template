// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CradleLightControlEditor : ModuleRules
{
    public CradleLightControlEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
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
                "DMXRuntime",
				"CradleLightControl",
				"Sockets",
				"Networking"

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
				"AssetTools",

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
