#pragma once


class PHYSICALOBJECTTRACKER_API FPhysicalObjectTrackingUtility
{
public:
	static bool GetTrackedDevicePositionAndRotation(int32 SteamVRDeviceId, FVector& Position, FQuat& Orientation);
	static bool FindDeviceIdFromSerialId(FString SerialId, int32& XRDeviceId);
	static bool FindSerialIdFromDeviceId(int32 SteamVRDeviceId, FString& SerialId);
	static void GetAllTrackingReferenceDeviceIds(TArray<int32>& DeviceIds);

	static FTransform GetRelativeTransformToBaseStation(const FVector& LocationTracker, const FQuat& RotationTracker, const FVector& LocationBaseStation, const FQuat& RotationBaseStation);
	static FTransform GetRelativeTransformToBaseStation(const FTransform& TransformTracker, const FTransform& TransformBaseStation);
};

