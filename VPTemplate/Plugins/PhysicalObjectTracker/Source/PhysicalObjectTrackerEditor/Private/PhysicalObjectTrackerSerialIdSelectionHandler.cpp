#include "PhysicalObjectTrackerSerialIdSelectionHandler.h"

#include "PhysicalObjectTrackerSerialId.h"
#include "DetectTrackerShakeTask.h"

#include "EditorStyleSet.h"
#include "IXRTrackingSystem.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTrackerEditor"

TSharedRef<FExtender> FPhysicalObjectTrackerSerialIdSelectionHandler::CreateMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    Extender->AddMenuExtension(
        "CommonAssetActions",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FPhysicalObjectTrackerSerialIdSelectionHandler::MenuExtenderImpl, SelectedAssets)
    );
    return Extender;
}

void FPhysicalObjectTrackerSerialIdSelectionHandler::MenuExtenderImpl(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
    if (SelectedAssets.Num() == 1 && SelectedAssets[0].GetClass() == UPhysicalObjectTrackerSerialId::StaticClass())
    {
        FAssetData serialIdAsset = SelectedAssets[0];
        MenuBuilder.BeginSection("Physical Object Tracking", LOCTEXT("ASSET_CONTEXT", "Physical Object Tracking"));
        {
            MenuBuilder.AddMenuEntry(
                LOCTEXT("SelectTracker", "Select Tracker"),
                LOCTEXT("SelectTrackerDesc", "Select the tracker that this Serial Id should represent."),
                FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.ViewOptions", "LevelEditor.ViewOptions.Small"),
                FUIAction(FExecuteAction::CreateLambda([this, serialIdAsset]() 
                    {
						UPhysicalObjectTrackerSerialId* serialId = Cast<UPhysicalObjectTrackerSerialId>(serialIdAsset.GetAsset());
						if(serialId != nullptr)
						{
							this->StartDeviceSelection(serialId);
						}
                    })),
                NAME_None,
                EUserInterfaceActionType::Button);
        }
        MenuBuilder.EndSection();
    }
}

void FPhysicalObjectTrackerSerialIdSelectionHandler::StartDeviceSelection(UPhysicalObjectTrackerSerialId* SerialIdAsset)
{
	if (!m_ShakeDetectTask || m_ShakeDetectTask->IsComplete())
	{
		FNotificationInfo spinner(LOCTEXT("DeviceSelectionShake", "Please shake the tracker to use for this component..."));
		spinner.bUseThrobber = true;
		spinner.ExpireDuration = 1e25f;
		spinner.FadeOutDuration = 0.5f;
		spinner.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("DeviceSelectionCancel", "Cancel"), LOCTEXT("DeviceSelectionCancel", "Cancel"),
			FSimpleDelegate::CreateRaw(this, &FPhysicalObjectTrackerSerialIdSelectionHandler::StopDeviceSelection)));
		m_ShakeProcessNotification = FSlateNotificationManager::Get().AddNotification(spinner);
		m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Pending);

		m_ShakeDetectTask = MakeUnique<FDetectTrackerShakeTask>();
		m_ShakeDetectTask->OnTaskFinished = FShakeTaskFinished::CreateLambda([SerialIdAsset, this](uint32 SelectedControllerId)
			{
				if (m_ShakeDetectTask->IsFailed())
				{
					m_ShakeProcessNotification->SetText(m_ShakeDetectTask->GetFailureReason());
					m_ShakeProcessNotification->SetExpireDuration(5.0f);
					m_ShakeProcessNotification->ExpireAndFadeout();
					m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
				}
				else
				{
					if (GEngine && GEngine->XRSystem)
					{
						SerialIdAsset->SetSerialId(GEngine->XRSystem->GetTrackedDevicePropertySerialNumber(SelectedControllerId));
						if(SerialIdAsset->MarkPackageDirty())
						{
							m_ShakeProcessNotification->SetText(LOCTEXT("DeviceSelectionSuccess", "Device selected successfully, Serial Id asset updated!"));
							m_ShakeProcessNotification->Fadeout();
							m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Success);
						}
						else
						{
							m_ShakeProcessNotification->SetText(LOCTEXT("DeviceSelectionFailed", "Device Serial Id updated but asset is not marked dirty. Manual intervention required."));
							m_ShakeProcessNotification->Fadeout();
							m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
						}
					}
					else
					{
						m_ShakeProcessNotification->SetText(LOCTEXT("DeviceSelectionFailed", "Device selection failed, no XR System available!"));
						m_ShakeProcessNotification->Fadeout();
						m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
					}
				}
			});
	}
	else
	{
		FNotificationInfo notification(LOCTEXT("CouldNotStartTrackerSelection", "Please finish or cancel the last tracker selection..."));
		notification.ExpireDuration = 15.0f;
		notification.Image = FSlateIconFinder::FindIcon("Icons.Error").GetIcon();

		FSlateNotificationManager::Get().AddNotification(notification);
	}
}

void FPhysicalObjectTrackerSerialIdSelectionHandler::StopDeviceSelection()
{
	if (m_ShakeProcessNotification)
	{
		m_ShakeProcessNotification->Fadeout();
		m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
		m_ShakeProcessNotification = nullptr;

		m_ShakeDetectTask->OnTaskFinished.Unbind();
	}
}

#undef LOCTEXT_NAMESPACE 