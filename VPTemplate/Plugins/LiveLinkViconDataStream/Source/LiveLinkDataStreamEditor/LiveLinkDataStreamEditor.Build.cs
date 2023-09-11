using UnrealBuildTool;
using System;
using System.IO;
using System.Text;
public class LiveLinkDataStreamEditor : ModuleRules
{

//    private string ThirdPartyPath
//    {
//        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty")); }
//    }

    public LiveLinkDataStreamEditor(ReadOnlyTargetRules Target) : base(Target)
  {
        //System.Console.WriteLine(ThirdPartyPath);
    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "LiveLinkDataStream" });
        PrivateDependencyModuleNames.AddRange(
      new string[]
      {
        "Core",
        "UnrealEd",
        "SlateCore",
        "Slate",
        "InputCore",
        "Projects",
        "LiveLink",
        "LiveLinkInterface",
        "TimeManagement"
      }
    );
    }
}
