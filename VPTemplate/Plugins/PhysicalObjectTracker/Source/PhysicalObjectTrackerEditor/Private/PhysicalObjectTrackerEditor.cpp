#include "PhysicalObjectTrackerEditor.h"

#include "ContentBrowserModule.h"
#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackingReferenceCalibrationHandler.h"
#include "PhysicalObjectTrackerSerialIdSelectionHandler.h"


#include "IXRTrackingSystem.h"
#include "PhysicalObjectTrackingComponentVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"


#include "SteamVRFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "PhysicalObjectTrackingReferencePoint.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTrackerEditor"

void FPhysicalObjectTrackerEditor::StartupModule()
{
	m_TrackingCalibrationHandler = MakeUnique<FPhysicalObjectTrackingReferenceCalibrationHandler>();
	m_TrackerSerialIdSelectionHandler = MakeUnique<FPhysicalObjectTrackerSerialIdSelectionHandler>();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(m_TrackingCalibrationHandler.Get(), &FPhysicalObjectTrackingReferenceCalibrationHandler::CreateMenuExtender));
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(m_TrackerSerialIdSelectionHandler.Get(), &FPhysicalObjectTrackerSerialIdSelectionHandler::CreateMenuExtender));

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

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPhysicalObjectTrackerEditor, PhysicalObjectTrackerEditor)