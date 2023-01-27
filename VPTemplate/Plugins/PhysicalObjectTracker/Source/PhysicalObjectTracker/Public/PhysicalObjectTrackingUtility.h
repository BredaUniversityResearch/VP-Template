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

	//https://forum.htc.com/topic/7214-positions-and-orientations-of-hmdtracker-and-world-coordinate-system
	//Fix the tracker's transform by rotating it.
	//Normal orientation of tracker in device space is (x: up, y:left, z:forward) if the tracker's forward is considered at the tip with the light and charging port beneath.
	static FTransform FixTrackerTransform(const FTransform& TrackerDeviceSpaceTransform);

	inline static const TMap<FString, FColor> BaseStationColors
	{
		{FString{"LHB-4DA74639"}, FColor::Red},		//Right front
		{FString{"LHB-397A56CC"}, FColor::Green},	//Right back
		{FString{"LHB-1BEC1CA4"}, FColor::Blue},	//Middle Right
		{FString{"LHB-2239FAC8"}, FColor::Yellow},
		{FString{"LHB-B6A41014"}, FColor::Magenta}, //Left back
		{FString{"LHB-2A1A0096"}, FColor::Cyan},
	};
};

