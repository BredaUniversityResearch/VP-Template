#pragma once


class PHYSICALOBJECTTRACKER_API FPhysicalObjectTrackingUtility
{
public:
	static bool GetTrackedDevicePositionAndRotation(int32 SteamVRDeviceId, FVector& Position, FQuat& Orientation);
	static bool FindDeviceIdFromSerialId(FString SerialId, int32& XRDeviceId);
	static bool FindSerialIdFromDeviceId(int32 SteamVRDeviceId, FString& SerialId);
	static void GetAllTrackingReferenceDeviceIds(TArray<int32>& DeviceIds);

	//https://forum.htc.com/topic/7214-positions-and-orientations-of-hmdtracker-and-world-coordinate-system
	//Fix the tracker's transform by rotating it.
	//Normal orientation of tracker in device space is (x: up, y:left, z:forward) if the tracker's forward is considered at the tip with the light and charging port beneath.
	static FTransform FixTrackerTransform(const FTransform& TrackerDeviceSpaceTransform);
};

