#include "PhysicalObjectTrackerEditor.h"

#include "DetectTrackerShakeTask.h"

#include "ContentBrowserModule.h"
#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackingReferenceCalibrationHandler.h"

#include "Framework/Notifications/NotificationManager.h"

#include "IXRTrackingSystem.h"
#include "PhysicalObjectTrackingComponentVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#include "Styling/SlateIconFinder.h"

#include "SteamVRFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "PhysicalObjectTrackingReferencePoint.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTrackerEditor"

void FPhysicalObjectTrackerEditor::StartupModule()
{
	m_TrackingCalibrationHandler = MakeUnique<FPhysicalObjectTrackingReferenceCalibrationHandler>();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(m_TrackingCalibrationHandler.Get(), &FPhysicalObjectTrackingReferenceCalibrationHandler::CreateMenuExtender));

	auto& TrackerEditorModule = FModuleManager::Get().GetModuleChecked<FPhysicalObjectTracker>("PhysicalObjectTracker");

	TrackerEditorModule.DeviceDetectionEvent.AddRaw(this, &FPhysicalObjectTrackerEditor::OnDeviceDetectionStarted);

	m_ComponentVisualizer = MakeShared<FPhysicalObjectTrackingComponentVisualizer>();
	ensure(GUnrealEd != nullptr);
	GUnrealEd->RegisterComponentVisualizer(UPhysicalObjectTrackingComponent::StaticClass()->GetFName(), m_ComponentVisualizer);
	if (m_ComponentVisualizer.IsValid())
	{
		m_ComponentVisualizer->OnRegister();
	}
}

void FPhysicalObjectTrackerEditor::ShutdownModule()
{
	m_TrackingCalibrationHandler = nullptr;
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UPhysicalObjectTrackingComponent::StaticClass()->GetFName());
	}
}

void FPhysicalObjectTrackerEditor::DebugDrawTrackingReferenceLocations(const UPhysicalObjectTrackingReferencePoint* PhysicalReferencePoint, const FTransform* WorldTransform)
{
	if (PhysicalReferencePoint != nullptr)
	{
		TArray<int32> deviceIds;
		USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::TrackingReference, deviceIds);

		for (int32 deviceId : deviceIds)
		{
			FVector position;
			FRotator rotation;
			if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(deviceId, position, rotation))
			{
				FTransform transform = PhysicalReferencePoint->ApplyTransformation(position, rotation.Quaternion());

				if (WorldTransform != nullptr)
				{
					FTransform::Multiply(&transform, &transform, WorldTransform);
				}
				DrawDebugBox(GWorld, transform.GetLocation(), FVector(8.0f, 8.0f, 8.0f), transform.GetRotation(), FColor::Magenta,
					false, -1, 0, 2);
			}
		}
	}
}

void FPhysicalObjectTrackerEditor::OnDeviceDetectionStarted(UPhysicalObjectTrackingComponent* TargetTrackingComponent)
{
	if (!m_ShakeDetectTask || m_ShakeDetectTask->IsComplete())
	{
		FNotificationInfo spinner(LOCTEXT("DeviceSelectionShake", "Please shake the tracker to use for this component..."));
		spinner.bUseThrobber = true;
		spinner.ExpireDuration = 1e25f;
		spinner.FadeOutDuration = 0.5f;
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
					m_ShakeProcessNotification->SetExpireDuration(5.0f);
					m_ShakeProcessNotification->ExpireAndFadeout();
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