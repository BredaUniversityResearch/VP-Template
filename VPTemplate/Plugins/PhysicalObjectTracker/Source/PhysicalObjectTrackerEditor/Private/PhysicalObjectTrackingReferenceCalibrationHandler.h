#pragma once

#include "UpdateTrackerCalibrationAsset.h"

class FUpdateTrackerCalibrationAsset;
class FPhysicalObjectTrackingReferenceCalibrationHandler
{
public:
	TSharedRef<FExtender> CreateMenuExtender(const TArray<FAssetData>& SelectedAssets);

private:
	void MenuExtenderImpl(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
		
	TUniquePtr<FUpdateTrackerCalibrationAsset> m_RunningCalibrationTask;
};

