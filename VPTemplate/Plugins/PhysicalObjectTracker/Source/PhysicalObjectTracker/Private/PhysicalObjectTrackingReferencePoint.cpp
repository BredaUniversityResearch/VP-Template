#include "PhysicalObjectTrackingReferencePoint.h"

#include "PhysicalObjectTrackingUtility.h"
#include "SteamVRFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

UPhysicalObjectTrackingReferencePoint::UPhysicalObjectTrackingReferencePoint(const FObjectInitializer& ObjectInitializer)
{}

void UPhysicalObjectTrackingReferencePoint::Tick(float DeltaTime)
{
	if(UpdateBaseStationOffsetsEachTick)
	{
		UpdateBaseStationOffsets();
	}
	else
	{
		UpdateBaseStationOffsetsDeltaTimeAccumulator += DeltaTime;
		const float SecondsBetweenUpdate = 1.f / BaseStationOffsetUpdatesPerSecond;
		if (UpdateBaseStationOffsetsDeltaTimeAccumulator > SecondsBetweenUpdate)
		{
			UpdateBaseStationOffsetsDeltaTimeAccumulator -= SecondsBetweenUpdate;
			UpdateBaseStationOffsets();
		}
	}
}

bool UPhysicalObjectTrackingReferencePoint::IsTickableInEditor() const
{
	return true;
}

void UPhysicalObjectTrackingReferencePoint::SetTrackerCalibrationTransform(const FTransform& InTransform)
{
	TrackerCalibrationTransform = InTransform;
}

void UPhysicalObjectTrackingReferencePoint::SetBaseStationCalibrationInfo(
	const FString& BaseStationSerialId,
	const FTransform& CalibrationTransform,
	const FColor& Color,
	bool StaticCalibration)
{
	if(!BaseStationSerialId.IsEmpty())
	{
		auto& info = BaseStationCalibrationInfo.FindOrAdd(BaseStationSerialId, { FTransform::Identity, false, Color });
		info.Transformation = CalibrationTransform;
		info.StaticallyCalibrated = StaticCalibration;
	}
}

const FTransform& UPhysicalObjectTrackingReferencePoint::GetTrackerCalibrationTransform() const
{
	return TrackerCalibrationTransform;
}

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationCalibrationTransform(const FString& BaseStationSerialId, FTransform& OutReferenceSpaceTransform, FTransform& OutRawTransform) const
{
	if (!BaseStationSerialId.IsEmpty())
	{
		if (const auto* baseStationTransform = BaseStationCalibrationInfo.Find(BaseStationSerialId))
		{
			OutRawTransform = baseStationTransform->Transformation;
			OutReferenceSpaceTransform = OutRawTransform.GetRelativeTransform(FPhysicalObjectTrackingUtility::FixTrackerTransform(TrackerCalibrationTransform));
			return true;
		}
	}

	OutRawTransform = FTransform::Identity;
	OutReferenceSpaceTransform = FTransform::Identity;
	return false;
}

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationColor(const FString& BaseStationSerialId, FColor& OutColor) const
{
	if(!BaseStationSerialId.IsEmpty())
	{
		if(const auto* calibrationInfo = BaseStationCalibrationInfo.Find(BaseStationSerialId))
		{
			OutColor = calibrationInfo->Color;
			return true;
		}
	}

	OutColor = FColor::Black;
	return false;
}

FTransform UPhysicalObjectTrackingReferencePoint::ApplyTransformation(const FTransform& TrackedTransform) const
{
	//Should use GetRelativeTransform as this returns transformA * inverse(transformB) where as
	//Transform.Inverse() simply inverts components separately and thus can not be used to undo transformations
	return TrackedTransform.GetRelativeTransform(FPhysicalObjectTrackingUtility::FixTrackerTransform(TrackerCalibrationTransform));
}

FTransform UPhysicalObjectTrackingReferencePoint::ApplyTransformation(
	const FVector& TrackedPosition,
	const FQuat& TrackedRotation) const
{
	return ApplyTransformation(FTransform(TrackedRotation, TrackedPosition));
}

FTransform UPhysicalObjectTrackingReferencePoint::GetTrackerReferenceSpaceTransform(const FTransform& TrackerCurrentTransform) const
{
	//1. For every base station calculate the offset transformation between the current transformation and the transformation at calibration.
	//2. Get the offset transformation between the current transformation of the tracker and the transformation at calibration.
	//(Is this to get the relative position of the tracker, the position of the tracker in reference-space)
	//3. Calculate the Tracker transformation by adding the offset transformation of the tracker and the offset transformation of the base stations.
	//4. Average the Tracker transformations, calculated for 

    /* Math formulas to calculate tracker transformation relative to the reference space
	 * using the offsets between the transformations of the base station at calibration and current time in SteamVR space.
	 *
	 * So = Steam Origin Calibration	= 0, 0,			0, 0,  0
	 * A = BaseStation Calibration 		= 10, 12		0, 90, 0
	 * B = Tracker Calibration Spot 	= 5, 3			0,  0, 0
	 * C = A - B						= 5, 9			0, 90, 0
	 * Oc = B							= 5, 3
	 *
	 * //Calculate back.
	 *
	 * Soc = Steam Origin Current 		= 5, 0, 		90, 0, 0
	 * D = BaseStation Current			= 15, 12		90, 90, 0
	 * E = Tracker Current				= 10, 8 		90, 0, 0
	 * F(BaseStation Offset) = A(BaseStation Calibration) - D(BaseStation Current)				= -5, 0			-90, 0, 0
	 *
	 *											B Might need to be mapped to the current SteamVR space somehow.
	 * G(TrackerOffset) = E(Tracker Current) - B(Tracker Calibration)							= 5, 5			90, 0, 0
	 * T'(TrackerRelativeTransform) = G(TrackerOffset) + F(BaseStation Offset)					= 0, 5, 		0, 0, 0
	 *
	 */

	//Done every tick
	//1. For every base station calculate the offset transformation between the current transformation and the transformation at calibration.
	//2. store the offset transformation.

	//Done in this function.
	//3. Get the offset transformation between the current transformation of the tracker and the transformation at calibration.
	//4. Calculate the Tracker transformation by adding the offset transformation of the tracker and the offset transformation of the base stations.

	const FTransform fixedTrackerTransform = FPhysicalObjectTrackingUtility::FixTrackerTransform(TrackerCurrentTransform);

	//BaseStationOffsets can be empty if no BaseStationSerialIds could be mapped to DeviceIds to avoid string lookups.
	//(Serial Id stay the same, while device ids can change in between sessions).
	//Should not happen as the base stations used should be the same as the ones at calibration and at least 1 needs to be connected for tracking.
	//So should be able to map at least one serial id to a device id, but as backup simply use the old method of tracking (non-relative to base stations)
	if(BaseStationOffsets.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(12345678, 5.f, FColor::Red, FString("Used fallback tracking method as no base station offsets have been calculated!"));
		return ApplyTransformation(fixedTrackerTransform);
	}

	//3. Get the offset transformation for the tracker.
	FVector averagedTrackerTranslation = FVector::Zero();
	FVector averagedTrackerRotation = FVector::Zero();
	uint32 averagedTrackerSampleCount = 0;

	for (const auto& baseStation : BaseStationOffsets)
	{
		const FTransform fixedRelativeTransform = fixedTrackerTransform * baseStation.Value;
		averagedTrackerTranslation += fixedRelativeTransform.GetTranslation();
		averagedTrackerRotation += fixedRelativeTransform.GetRotation().Euler();
		averagedTrackerSampleCount++;
	}

	averagedTrackerTranslation /= averagedTrackerSampleCount;
	averagedTrackerRotation /= averagedTrackerSampleCount;
	const FTransform averagedTrackerTransformRelativeRaw(
		FQuat::MakeFromEuler(averagedTrackerRotation),
		averagedTrackerTranslation);

	return ApplyTransformation(averagedTrackerTransformRelativeRaw);
}

bool UPhysicalObjectTrackingReferencePoint::HasMappedAllBaseStations() const
{
	//TODO: might not be entirely correct if for some reason the base station Id changes mid session?
	return BaseStationIdToCalibrationTransforms.Num() == BaseStationCalibrationInfo.Num() && !BaseStationCalibrationInfo.IsEmpty();
}

bool UPhysicalObjectTrackingReferencePoint::MapBaseStationIds()
{
	if(HasMappedAllBaseStations())
	{
		return true;
	}

	for (const auto& baseStation : BaseStationCalibrationInfo)
	{
		int32 baseStationId = -1;
		if (FPhysicalObjectTrackingUtility::FindDeviceIdFromSerialId(baseStation.Key, baseStationId))
		{
			BaseStationIdToCalibrationTransforms.Add(baseStationId, baseStation.Value.Transformation);
		}
	}

	return HasMappedAllBaseStations();
}

void UPhysicalObjectTrackingReferencePoint::UpdateBaseStationOffsets()
{
	//If not all of the calibration transforms have been mapped to a device id,
	//try to map the base stations calibration transforms to a device id.
	if(!HasMappedAllBaseStations())
	{
		MapBaseStationIds();
	}

	//Get the device ids of all the currently connected base stations.
	TArray<int32> currentBaseStationIds{};
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(currentBaseStationIds);

	//Sample the offsets between the calibration transform and the current transform
	for (const auto baseStation : BaseStationIdToCalibrationTransforms)
	{
		if (currentBaseStationIds.Contains(baseStation.Key))	//Only sample the base station if it is currently connected (valid)
		{
			FVector currentBaseStationPosition;
			FQuat currentBaseStationRotation;
			if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(baseStation.Key, currentBaseStationPosition, currentBaseStationRotation))
			{
				//1. Calculate the offset transformation between the current transformation and the transformation at calibration.
				//Should use GetRelativeTransform as this returns leftTransform * inverse(rightTransform) where as
				//Transform.Inverse() simply inverts components separately and thus can not be used to undo transformations. (check function declaration)
				const FTransform currentBaseStationTransform(currentBaseStationRotation, currentBaseStationPosition);
				const FTransform offset = currentBaseStationTransform.GetRelativeTransformReverse(baseStation.Value);
				BaseStationOffsets.FindOrAdd(baseStation.Key) = offset;
			}
		}
	}

}

int32 UPhysicalObjectTrackingReferencePoint::GetMinBaseStationsCalibrated() const
{
	return MinNumBaseStationsCalibrated;
}

int32 UPhysicalObjectTrackingReferencePoint::GetMinBaseStationsCalibratedStatically() const
{
	return FMath::Min(MinNumBaseStationsCalibratedStatically, MinNumBaseStationsCalibrated);
}

void UPhysicalObjectTrackingReferencePoint::PostLoad()
{
	UDataAsset::PostLoad();
	MapBaseStationIds();
}

void UPhysicalObjectTrackingReferencePoint::PostInitProperties()
{
	UDataAsset::PostInitProperties();
	MapBaseStationIds();
}

void UPhysicalObjectTrackingReferencePoint::PostReinitProperties()
{
	UDataAsset::PostReinitProperties();
	MapBaseStationIds();
}

#if WITH_EDITOR
void UPhysicalObjectTrackingReferencePoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	MapBaseStationIds();
}
#endif