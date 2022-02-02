#include "PhysicalObjectTrackingUtility.h"

#include "IXRTrackingSystem.h"
#include "SteamVRFunctionLibrary.h"

namespace {
	FString GetTrackerSerialNo(int32 DeviceId)
	{
		if (GEngine)
		{
			return GEngine->XRSystem->GetTrackedDevicePropertySerialNumber(DeviceId);
		}
		return "Invalid Serial Number";
	}
}

bool FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(int32 SteamVRDeviceId, FVector& Position, FQuat& Orientation)
{
	FRotator trackedOrientation;
	if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(SteamVRDeviceId, Position, trackedOrientation))
	{
		Orientation = trackedOrientation.Quaternion();
		return true;
	}
	return false;
}
