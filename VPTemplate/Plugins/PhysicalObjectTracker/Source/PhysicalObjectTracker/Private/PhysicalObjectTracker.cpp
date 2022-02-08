#include "PhysicalObjectTracker.h"

#include "SteamVRFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "PhysicalObjectTrackingReferencePoint.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTracker"

DEFINE_LOG_CATEGORY(LogPhysicalObjectTracker);

void FPhysicalObjectTracker::StartupModule()
{
}

void FPhysicalObjectTracker::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FPhysicalObjectTracker::DebugDrawTrackingReferenceLocations(const UPhysicalObjectTrackingReferencePoint* ReferencePoint)
{
	if (ReferencePoint != nullptr)
	{
		TArray<int32> deviceIds;
		USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::TrackingReference, deviceIds);

		for (int32 deviceId : deviceIds)
		{
			FVector position;
			FRotator rotation;
			if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(deviceId, position, rotation))
			{
				FTransform transform = ReferencePoint->ApplyTransformation(position, rotation.Quaternion());

				DrawDebugBox(GWorld, transform.GetLocation(), FVector(8.0f, 8.0f, 8.0f), transform.GetRotation(), FColor::Magenta, 
					false, -1, 0, 2);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhysicalObjectTracker, PhysicalObjectTracker)