#pragma once


class PHYSICALOBJECTTRACKER_API FPhysicalObjectTrackingUtility
{
public:
	static bool GetTrackedDevicePositionAndRotation(int32 SteamVRDeviceId, FVector& Position, FQuat& Orientation);
};

