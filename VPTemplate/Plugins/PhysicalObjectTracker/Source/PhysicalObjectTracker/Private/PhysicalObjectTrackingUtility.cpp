#include "PhysicalObjectTrackingUtility.h"

#include "IXRTrackingSystem.h"
#include "SteamVRFunctionLibrary.h"

namespace {
	FString GetTrackerSerialNo(int32 DeviceId)
	{
		GEngine->XRSystem->GetTrackedDevicePropertySerialNumber(DeviceId);
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
