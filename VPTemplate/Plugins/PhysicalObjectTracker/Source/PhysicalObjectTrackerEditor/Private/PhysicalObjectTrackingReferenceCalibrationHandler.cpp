#include "PhysicalObjectTrackingReferenceCalibrationHandler.h"

#include "EditorStyleSet.h"
#include "PhysicalObjectTrackingReferencePoint.h"
#include "UpdateTrackerCalibrationAsset.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Containers/RingBuffer.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTrackerEditor"

TSharedRef<FExtender> FPhysicalObjectTrackingReferenceCalibrationHandler::CreateMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension(
		"CommonAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FPhysicalObjectTrackingReferenceCalibrationHandler::MenuExtenderImpl, SelectedAssets)
	);
	return Extender;
}

void FPhysicalObjectTrackingReferenceCalibrationHandler::MenuExtenderImpl(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	if (SelectedAssets.Num() == 1 && SelectedAssets[0].GetClass() == UPhysicalObjectTrackingReferencePoint::StaticClass())
	{
		MenuBuilder.BeginSection("Physical Object Tracking", LOCTEXT("ASSET_CONTEXT", "Physical Object Tracking"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("SetFromTracker", "Set From Tracker"),
				LOCTEXT("SetFromTrackerDesc", "Sets the calibration point data from the selected tracker."),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.ViewOptions", "LevelEditor.ViewOptions.Small"),
				FUIAction(FExecuteAction::CreateLambda([this, SelectedAssets]()
					{
						if (m_RunningCalibrationTask != nullptr && !m_RunningCalibrationTask->IsCompleted())
						{
							return;
						}

						for (const FAssetData& asset : SelectedAssets)
						{
							UPhysicalObjectTrackingReferencePoint* referencePoint = Cast<UPhysicalObjectTrackingReferencePoint>(asset.GetAsset());
							if (referencePoint != nullptr)
							{
								m_RunningCalibrationTask = MakeUnique<FUpdateTrackerCalibrationAsset>(referencePoint);
							}
						}
					})),
				NAME_None,
						EUserInterfaceActionType::Button);
		}
		MenuBuilder.EndSection();
	}
}

#undef LOCTEXT_NAMESPACE 
