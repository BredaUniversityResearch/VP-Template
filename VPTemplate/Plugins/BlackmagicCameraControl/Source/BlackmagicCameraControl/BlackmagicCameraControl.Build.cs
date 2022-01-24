
using System.IO;
using UnrealBuildTool;

public class BlackmagicCameraControl : ModuleRules
{
	public BlackmagicCameraControl(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		if (Target.Platform != UnrealTargetPlatform.Win64)
			throw new BuildException("Target platform is not supported for BlackMagicCameraControl");

		// These parameters are mandatory for winrt support
			bEnableExceptions = true;
		bUseUnity = false;
		CppStandard = CppStandardVersion.Cpp17;
		PublicSystemLibraries.AddRange(new string[] { "shlwapi.lib", "runtimeobject.lib" });
		PrivateIncludePaths.Add(Path.Combine(Target.WindowsPlatform.WindowsSdkDir,
			"Include",
			Target.WindowsPlatform.WindowsSdkVersion,
			"cppwinrt"));

		if (Target.Configuration == UnrealTargetConfiguration.Debug ||
		    Target.Configuration == UnrealTargetConfiguration.DebugGame)
		{
			PrivateDefinitions.Add("WINRT_NATVIS");
		}

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
				"Engine"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CinematicCamera"
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
