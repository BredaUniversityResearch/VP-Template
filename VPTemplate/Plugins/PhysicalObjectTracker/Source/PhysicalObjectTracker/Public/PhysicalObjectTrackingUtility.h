#pragma once


class PHYSICALOBJECTTRACKER_API FPhysicalObjectTrackingUtility
{
public:
	static bool GetTrackedDevicePositionAndRotation(int32 SteamVRDeviceId, FVector& Position, FQuat& Orientation);
	static bool FindDeviceIdFromSerialId(FString SerialId, int32& XRDeviceId);
	static bool FindSerialIdFromDeviceId(int32 SteamVRDeviceId, FString& SerialId);

	static void GetAllTrackingReferenceDeviceIds(TArray<int32>& DeviceIds);
};

