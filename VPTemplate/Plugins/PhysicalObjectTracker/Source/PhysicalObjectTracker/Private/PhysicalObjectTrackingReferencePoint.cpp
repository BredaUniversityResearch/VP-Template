#include "PhysicalObjectTrackingReferencePoint.h"

#include "PhysicalObjectTrackingUtility.h"
#include "SteamVRFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

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

	//Should not happen as the base stations used should be the same as the ones at calibration (Serial Id stay same)
	//and at least 1 needs to be connected for tracking. But can happen if the base stations have not been mapped yet.
	//TODO: consider instead of applying the transformation normally based on the calibration transforms, to try and map the base stations first if not all of them have been mapped at the start of this function.
	if(numBaseStationSamples == 0)
	{
		return ApplyTransformation(TrackerCurrentTransform.GetLocation(), TrackerCurrentTransform.GetRotation());
	}

	//2. Average the offset transformations.
	averagedBaseStationOffsetTranslation /= numBaseStationSamples;
	averagedBaseStationOffsetRotation /= numBaseStationSamples;

	const FTransform baseStationOffset(FQuat::MakeFromEuler(averagedBaseStationOffsetRotation), averagedBaseStationOffsetTranslation);
	const FTransform trackerOffset = TrackerCurrentTransform * FPhysicalObjectTrackingUtility::FixTrackerTransform(TrackerCalibrationTransform).Inverse();	//3. Get the offset transformation for the tracker.
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

void UPhysicalObjectTrackingReferencePoint::UpdateRuntimeDataIfNeeded()
{
	MapBaseStationIds();
}

bool UPhysicalObjectTrackingReferencePoint::HasMappedAllBaseStations() const
{
	//TODO: might not be entirely correct if for some reason the base station Id changes mid session?
	return BaseStationIdToCalibrationTransform.Num() == BaseStationCalibrationTransforms.Num() && !BaseStationCalibrationTransforms.IsEmpty();
}

bool UPhysicalObjectTrackingReferencePoint::MapBaseStationIds()
{
	if(HasMappedAllBaseStations())
	{
		return true;
	}

	for (const auto& baseStation : BaseStationCalibrationTransforms)
	{
		int32 baseStationId = -1;
		if (FPhysicalObjectTrackingUtility::FindDeviceIdFromSerialId(baseStation.Key, baseStationId))
		{
			BaseStationIdToCalibrationTransform.Add(baseStationId, baseStation.Value);
		}
	}

	return HasMappedAllBaseStations();
}

void UPhysicalObjectTrackingReferencePoint::PostLoad()
{
	Super::PostLoad();
	UpdateRuntimeDataIfNeeded();
}

void UPhysicalObjectTrackingReferencePoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateRuntimeDataIfNeeded();
}

void UPhysicalObjectTrackingReferencePoint::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateRuntimeDataIfNeeded();
}

void UPhysicalObjectTrackingReferencePoint::PostReinitProperties()
{
	Super::PostReinitProperties();
	UpdateRuntimeDataIfNeeded();
}
