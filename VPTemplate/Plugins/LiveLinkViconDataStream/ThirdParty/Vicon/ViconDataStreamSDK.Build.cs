
using System;
using System.IO;
using UnrealBuildTool;
using System.Security.Cryptography;
using System.Text;

public class ViconDataStreamSDK : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "")); }
    }

    public ViconDataStreamSDK( ReadOnlyTargetRules Target) : base( Target )
    {
        Type = ModuleType.External;
       // if ((Target.Platform == UnrealTargetPlatform.Win64))
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "DataStreamSDK/");
            PublicIncludePaths.Add(LibrariesPath);
            PublicAdditionalLibraries.Add(LibrariesPath + "ViconDataStreamSDK_CPP.lib");
            //PublicAdditionalLibraries.Add(LibrariesPath + "HMDFusionUtils.lib");
            PublicDelayLoadDLLs.Add(LibrariesPath + "ViconDataStreamSDK_CPP.dll");
        }
    }
}
