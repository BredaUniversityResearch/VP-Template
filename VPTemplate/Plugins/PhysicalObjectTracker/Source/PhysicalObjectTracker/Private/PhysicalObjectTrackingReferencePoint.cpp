#include "PhysicalObjectTrackingReferencePoint.h"

#include "PhysicalObjectTrackingUtility.h"

void UPhysicalObjectTrackingReferencePoint::SetTrackerCalibrationTransform(const FTransform& TrackerOriginTransform)
{
	check(!IsRunningGame());
	CalibrationTrackerTransform = TrackerOriginTransform;
}

void UPhysicalObjectTrackingReferencePoint::SetBaseStationCalibrationTransforms(const TMap<int32, FTransform>& Offsets)
{
	BaseStationOffsetsTransformsAtCalibration.Empty();
	for (const auto& baseStation : Offsets)
	{
		FString baseStationSerialId;
		if (FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(baseStation.Key, baseStationSerialId))
		{
			FTransform baseStationTransform = baseStation.Value;

			BaseStationOffsetsTransformsAtCalibration.Add(baseStationSerialId, baseStationTransform);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(
				1, 30.0f, FColor::Red,
				FString::Format(TEXT("Could not get Device Serial Id of base station with Device Id: \"{0}\""),
					FStringFormatOrderedArguments({ baseStation.Key })));
		}
	}
}

const FTransform& UPhysicalObjectTrackingReferencePoint::GetTrackerCalibrationTransform() const
{
	return CalibrationTrackerTransform;
}

FTransform UPhysicalObjectTrackingReferencePoint::ApplyTransformation(const FVector& TrackedPosition,
	const FQuat& TrackedRotation) const
{

	return FTransform(TrackedRotation, TrackedPosition);
	/*static const FMatrix deviceToWorldSpace = 
		FRotationMatrix::Make(FQuat(FVector::YAxisVector, 
			FMath::DegreesToRadians(90))) * FScaleMatrix::Make(FVector(1.0f, -1.0f, -1.0f));

	FQuat orientation = TrackedRotation * GetNeutralRotationInverse();

	FRotator rotationInversionFix = FRotator(orientation);
	if (InvertPitchRotation)
	{
		rotationInversionFix.Pitch = -rotationInversionFix.Pitch;
	}
	if (InvertYawRotation)
	{
		rotationInversionFix.Yaw = -rotationInversionFix.Yaw;
	}
	if (InvertRollRotation)
	{
		rotationInversionFix.Roll = -rotationInversionFix.Roll;
	}
	orientation = rotationInversionFix.Quaternion();

	const FVector devicePosition = GetNeutralRotationInverse() * (TrackedPosition - GetNeutralOffset());
	const FVector4 position = deviceToWorldSpace.TransformPosition(devicePosition);
	return FTransform(orientation, position);*/
}

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationWorldTransform(const FString& BaseStationSerialId, FTransform& Result) const
{
	if (const FTransform* calibration = BaseStationOffsetsTransformsAtCalibration.Find(BaseStationSerialId))
	{
		Result = FTransform(calibration->GetRotation(), calibration->GetLocation());
		return true;
	}
	return false;
}

FTransform UPhysicalObjectTrackingReferencePoint::GetAveragedTransform(const TArray<FBaseStationOffset>& OffsetDifferences)
{
	FVector averageLocation = FVector::ZeroVector;
	FVector averageRotationEuler = FVector::ZeroVector;

	for(const auto& offset : OffsetDifferences)
	{
		averageLocation += offset.Position;
		averageRotationEuler += offset.Rotation.Euler();
	}
	averageLocation = averageLocation / OffsetDifferences.Num();
	averageRotationEuler = averageRotationEuler / OffsetDifferences.Num();

	return FTransform(FQuat::MakeFromEuler(averageRotationEuler), averageLocation);
}
