#pragma once


class PHYSICALOBJECTTRACKER_API FPhysicalObjectTrackingUtility
{
public:
	static bool GetTrackedDevicePositionAndRotation(int32 SteamVRDeviceId, FVector& Position, FQuat& Orientation);
	static bool FindDeviceIdFromSerialId(FString SerialId, int32& XRDeviceId);
	static bool FindSerialIdFromDeviceId(int32 SteamVRDeviceId, FString& SerialId);
	static void GetAllTrackingReferenceDeviceIds(TArray<int32>& DeviceIds);
	
	//Fix the tracker's transform by rotating it, default orientation of tracker is (x: up, y:left, z:forward)
	//when tracker's forward is considered the wing with the light and charging port beneath it.
	static FTransform FixTrackerTransform(const FTransform& TrackerDeviceSpaceTransform);
};

