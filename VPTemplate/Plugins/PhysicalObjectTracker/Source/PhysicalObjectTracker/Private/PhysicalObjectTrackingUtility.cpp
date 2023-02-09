#include "PhysicalObjectTrackingUtility.h"

#include "IXRTrackingSystem.h"
#include "SteamVRFunctionLibrary.h"

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

bool FPhysicalObjectTrackingUtility::FindDeviceIdFromSerialId(FString SerialId, int32& XRDeviceId)
{
	if (GEngine && GEngine->XRSystem && !SerialId.IsEmpty())
	{
		auto XR = GEngine->XRSystem;

		TArray<int32> DeviceIds;
		XR->EnumerateTrackedDevices(DeviceIds);

		for (auto DeviceId : DeviceIds)
		{
			auto DeviceSerial = XR->GetTrackedDevicePropertySerialNumber(DeviceId);
			if (DeviceSerial == SerialId)
			{
				XRDeviceId = DeviceId;
				return true;
			}

		}
	}

	return false;
}

bool FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(int32 SteamVRDeviceId, FString& SerialId)
{
	if(GEngine && GEngine->XRSystem && SteamVRDeviceId != -1)
	{
		SerialId = GEngine->XRSystem->GetTrackedDevicePropertySerialNumber(SteamVRDeviceId);
		return(!SerialId.IsEmpty());
	}
	return false;
}

void FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(TArray<int32>& DeviceIds)
{
	USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::TrackingReference, DeviceIds);
}

FTransform FPhysicalObjectTrackingUtility::FixTrackerTransform(const FTransform& TrackerDeviceSpaceTransform)
{
	//https://forum.htc.com/topic/7214-positions-and-orientations-of-hmdtracker-and-world-coordinate-system
	//Fix the tracker's transform by rotating it.
	//Normal orientation of tracker in device space is (x: up, y:left, z:forward) if
	//the tracker's forward is considered at the tip with the light and charging port beneath.
	//https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Public/Math/TransformVectorized.h
	//This header explains how transformations apply to each other, specifically what order to place them for the desired outcome.
	const FTransform TrackerRotationFix = FTransform(FQuat::MakeFromEuler(FVector(0.f, 90.f, 180.f)));
	return TrackerRotationFix * TrackerDeviceSpaceTransform;
}