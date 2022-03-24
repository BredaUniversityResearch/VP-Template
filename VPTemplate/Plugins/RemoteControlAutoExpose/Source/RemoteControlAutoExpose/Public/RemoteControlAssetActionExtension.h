#pragma once

#include "CoreMinimal.h"

class URemoteControlPreset;

class REMOTECONTROLAUTOEXPOSE_API FRemoteControlAssetActionExtension 
{
public:
	TSharedRef<FExtender> CreateMenuExtender(const TArray<FAssetData>& SelectedAssets);

private:
	void MenuExtenderImpl(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

	void PopulateAssetFromCurrentScene(URemoteControlPreset* TargetAsset) const;
};

