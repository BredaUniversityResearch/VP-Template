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

FTransform FPhysicalObjectTrackingUtility::GetRelativeTransform(
	const FVector& LocationA, 
	const FQuat& OrientationA, 
	const FVector& LocationB, 
	const FQuat& OrientationB)
{
	const FVector translationAtoB = LocationB - LocationA;
	const FQuat rotationAtoB = OrientationB * OrientationA.Inverse();
	return FTransform(rotationAtoB, translationAtoB);
}

FTransform FPhysicalObjectTrackingUtility::GetRelativeTransform(
	const FTransform& TransformationA, 
	const FTransform& TransformationB)
{
	return GetRelativeTransform(
		TransformationA.GetLocation(), TransformationA.GetRotation(),
		TransformationB.GetLocation(), TransformationB.GetRotation());
}