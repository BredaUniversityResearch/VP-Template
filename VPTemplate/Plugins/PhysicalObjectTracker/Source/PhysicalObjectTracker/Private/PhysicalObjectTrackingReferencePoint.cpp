#include "PhysicalObjectTrackingReferencePoint.h"

#include "PhysicalObjectTrackingUtility.h"
#include "SteamVRFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

void UPhysicalObjectTrackingReferencePoint::PostInitProperties()
{
	Super::PostInitProperties();
	//TODO: add event to delegates that reconfigure/reload the VR system, potentially causing device ids to be invalidated/changed.
	MapBaseStationIds();
}

void UPhysicalObjectTrackingReferencePoint::SetTrackerCalibrationTransform(const FTransform& InTransform)
{
	TrackerCalibrationTransform = InTransform;
}

void UPhysicalObjectTrackingReferencePoint::SetBaseStationCalibrationTransform(
	const FString& BaseStationSerialId,
	const FTransform& OffsetCalibrationTransform,
	const FColor& Color,
	bool StaticCalibration)
{
	if(!BaseStationSerialId.IsEmpty())
	{
		BaseStationCalibrationTransforms.Add(BaseStationSerialId, OffsetCalibrationTransform);
		BaseStationCalibrationInfo.Add(BaseStationSerialId, { StaticCalibration, Color });
	}
}

void UPhysicalObjectTrackingReferencePoint::ResetBaseStationOffsets()
{
	BaseStationCalibrationTransforms.Empty();
	BaseStationCalibrationInfo.Empty();
}

const FTransform& UPhysicalObjectTrackingReferencePoint::GetTrackerCalibrationTransform() const
{
	return TrackerCalibrationTransform;
}

const TMap<FString, FTransform>& UPhysicalObjectTrackingReferencePoint::GetBaseStationCalibrationTransforms() const
{
	return BaseStationCalibrationTransforms;
}

FTransform UPhysicalObjectTrackingReferencePoint::ApplyTransformation(
	const FVector& TrackedPosition,
	const FQuat& TrackedRotation) const
{
	return FTransform(TrackedRotation, TrackedPosition) * FPhysicalObjectTrackingUtility::FixTrackerTransform(TrackerCalibrationTransform).Inverse();
}

FTransform UPhysicalObjectTrackingReferencePoint::GetTrackerWorldTransform(const FTransform& TrackerCurrentTransform) const
{
	//1. For every base station calculate the offset transformation between the current transformation and the transformation at calibration.
	//2. Average the offset transformations.
	//3. Get the offset transformation between the current transformation of the tracker and the transformation at calibration.
	//4. Calculate the Tracker transformation by adding the offset transformation of the tracker and the offset transformation of the base stations.

	TArray<int32> currentBaseStationIds{};
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(currentBaseStationIds);

	FVector averagedBaseStationOffsetTranslation = FVector::ZeroVector;
	FVector averagedBaseStationOffsetRotation = FVector::ZeroVector;
	int32 numBaseStationSamples = 0;
	for(const auto baseStation : BaseStationIdToCalibrationTransform)
	{
		if(currentBaseStationIds.Contains(baseStation.Key))	//Only sample the base station if it is currently connected (valid)
		{
			FVector currentBaseStationPosition;
			FQuat currentBaseStationRotation;
			if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(baseStation.Key, currentBaseStationPosition, currentBaseStationRotation))
			{
				//1. Calculate the offset transformation between the current transformation and the transformation at calibration.
				const FTransform offset = FTransform(currentBaseStationRotation, currentBaseStationPosition) * baseStation.Value.Inverse();
				averagedBaseStationOffsetTranslation += offset.GetTranslation();
				averagedBaseStationOffsetRotation += offset.GetRotation().Euler(); 
				++numBaseStationSamples;
			}
		}
	}

	if(numBaseStationSamples == 0)
	{
		//TODO: simply return the normal way off applying the transformation.
		return FTransform::Identity;
	}

	//2. Average the offset transformations.
	averagedBaseStationOffsetTranslation /= numBaseStationSamples;
	averagedBaseStationOffsetRotation /= numBaseStationSamples;

	const FTransform baseStationOffset(FQuat::MakeFromEuler(averagedBaseStationOffsetRotation), averagedBaseStationOffsetTranslation);
	const FTransform trackerOffset = TrackerCurrentTransform * TrackerCalibrationTransform.Inverse();	//3. Get the offset transformation for the tracker.
	return trackerOffset * baseStationOffset;
}

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationWorldTransform(const FString& BaseStationSerialId, FTransform& WorldTransform) const
{
	if(!BaseStationSerialId.IsEmpty())
	{
		if(const FTransform* baseStationTransform = BaseStationCalibrationTransforms.Find(BaseStationSerialId))
		{
			WorldTransform = *baseStationTransform * FPhysicalObjectTrackingUtility::FixTrackerTransform(TrackerCalibrationTransform).Inverse();;
			return true;
		}
	}

	WorldTransform = FTransform::Identity;
	return false;
}


void UPhysicalObjectTrackingReferencePoint::MapBaseStationIds()
{
	for (const auto& baseStation : BaseStationCalibrationTransforms)
	{
		int32 baseStationId = -1;
		if (FPhysicalObjectTrackingUtility::FindDeviceIdFromSerialId(baseStation.Key, baseStationId))
		{
			BaseStationIdToCalibrationTransform.Add(baseStationId, baseStation.Value);
		}
	}
}