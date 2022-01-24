#include "PhysicalObjectTrackerEditor.h"

#include "DetectTrackerShakeTask.h"

#include "ContentBrowserModule.h"
#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackingReferenceCalibrationHandler.h"

#include "Framework/Notifications/NotificationManager.h"

#include "IXRTrackingSystem.h"


#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTrackerEditor"

void FPhysicalObjectTrackerEditor::StartupModule()
{
	m_TrackingCalibrationHandler = MakeUnique<FPhysicalObjectTrackingReferenceCalibrationHandler>();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(m_TrackingCalibrationHandler.Get(), &FPhysicalObjectTrackingReferenceCalibrationHandler::CreateMenuExtender));

	auto& TrackerEditorModule = FModuleManager::Get().GetModuleChecked<FPhysicalObjectTracker>("PhysicalObjectTracker");

	TrackerEditorModule.DeviceDetectionEvent.AddRaw(this, &FPhysicalObjectTrackerEditor::OnDeviceDetectionStarted);

}

void FPhysicalObjectTrackerEditor::ShutdownModule()
{
	m_TrackingCalibrationHandler = nullptr;
}

void FPhysicalObjectTrackerEditor::OnDeviceDetectionStarted(UPhysicalObjectTrackingComponent* TargetTrackingComponent)
{
	if (!m_ShakeDetectTask || m_ShakeDetectTask->IsComplete())
	{
		FNotificationInfo spinner(LOCTEXT("DeviceSelectionShake", "Please shake the tracker to use for this component..."));
		spinner.bUseThrobber = true;
		spinner.ExpireDuration = 1e25f;
		spinner.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("DeviceSelectionCancel", "Cancel"), LOCTEXT("DeviceSelectionCancel", "Cancel"),
			FSimpleDelegate::CreateRaw(this, &FPhysicalObjectTrackerEditor::StopDeviceSelection)));
		m_ShakeProcessNotification = FSlateNotificationManager::Get().AddNotification(spinner);
		m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Pending);

		m_ShakeDetectTask = MakeUnique<FDetectTrackerShakeTask>();
		m_ShakeDetectTask->OnTaskFinished = FShakeTaskFinished::CreateLambda([TargetTrackingComponent, this](uint32 SelectedControllerId)
			{
				if (m_ShakeDetectTask->IsFailed())
				{
					m_ShakeProcessNotification->SetText(m_ShakeDetectTask->GetFailureReason());
					m_ShakeProcessNotification->SetExpireDuration(60.0f);
					m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Fail);
				}
				else
				{
					TargetTrackingComponent->CurrentTargetDeviceId = SelectedControllerId;
					if (GEngine && GEngine->XRSystem)
					{
						TargetTrackingComponent->SerialId = GEngine->XRSystem->GetTrackedDevicePropertySerialNumber(SelectedControllerId);
					}
					m_ShakeProcessNotification->SetText(LOCTEXT("DeviceSelectionSuccess", "Device selected successfully!"));
					m_ShakeProcessNotification->Fadeout();
					m_ShakeProcessNotification->SetCompletionState(SNotificationItem::CS_Success);
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

void FPhysicalObjectTrackerEditor::StopDeviceSelection()
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

IMPLEMENT_MODULE(FPhysicalObjectTrackerEditor, PhysicalObjectTrackerEditor)