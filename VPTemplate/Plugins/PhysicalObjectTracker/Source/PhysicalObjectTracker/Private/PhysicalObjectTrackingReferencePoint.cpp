#include "PhysicalObjectTrackingReferencePoint.h"

#include "SteamVRFunctionLibrary.h"

void UPhysicalObjectTrackingReferencePoint::SetTrackerCalibrationTransform(const FTransform& InTransform)
{
	TrackerCalibrationTransform = InTransform;
}

void UPhysicalObjectTrackingReferencePoint::SetBaseStationOffsetToOrigin(
	const FString& BaseStationSerialId,
	const FTransform& OffsetCalibrationTransform,
	const FColor& Color,
	bool StaticCalibration)
{
	if(!BaseStationSerialId.IsEmpty())
	{
		BaseStationOffsetCalibrationTransforms.Add(BaseStationSerialId, OffsetCalibrationTransform);
		BaseStationCalibrationInfo.Add(BaseStationSerialId, { StaticCalibration, Color });
	}
}

void UPhysicalObjectTrackingReferencePoint::ResetBaseStationOffsets()
{
	BaseStationOffsetCalibrationTransforms.Empty();
	BaseStationCalibrationInfo.Empty();
}

const FTransform& UPhysicalObjectTrackingReferencePoint::GetTrackerCalibrationTransform() const
{
	return TrackerCalibrationTransform;
}

const TMap<FString, FTransform>& UPhysicalObjectTrackingReferencePoint::GetBaseStationOffsetCalibrationTransforms() const
{
	return BaseStationOffsetCalibrationTransforms;
}

FTransform UPhysicalObjectTrackingReferencePoint::ApplyTransformation(const FVector& TrackedPosition,
	const FQuat& TrackedRotation) const
{
	static const FMatrix deviceToWorldSpace = 
		FRotationMatrix::Make(FQuat(FVector::YAxisVector, 
			FMath::DegreesToRadians(90))) * FScaleMatrix::Make(FVector(1.0f, -1.0f, -1.0f));

	FQuat orientation = TrackedRotation * TrackerCalibrationTransform.GetRotation().Inverse();

	/*FRotator rotationInversionFix = FRotator(orientation);
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
	orientation = rotationInversionFix.Quaternion();*/

	const FVector devicePosition = TrackerCalibrationTransform.GetRotation().Inverse() * (TrackedPosition - TrackerCalibrationTransform.GetLocation());
	const FVector4 position = deviceToWorldSpace.TransformPosition(devicePosition);
	return FTransform(orientation, position);
}

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationWorldTransform(const FString& BaseStationSerialId, FTransform& WorldTransform) const
{
	static const FMatrix deviceToWorldSpace = FMatrix::Identity;
		/*FRotationMatrix::Make(FQuat(FVector::YAxisVector,
			FMath::DegreesToRadians(90))) * FScaleMatrix::Make(FVector(1.0f, -1.0f, -1.0f));*/
	
	if(!BaseStationSerialId.IsEmpty())
	{
		if(const FTransform* baseStationOffset = BaseStationOffsetCalibrationTransforms.Find(BaseStationSerialId))
		{
			FQuat orientation = baseStationOffset->GetRotation(); //The rotation is stored relative to the tracker's neutral rotation. 

			/*FRotator rotationInversionFix = FRotator(orientation);
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
			orientation = rotationInversionFix.Quaternion();*/

			const FVector4 devicePosition = baseStationOffset->GetLocation();
			WorldTransform = FTransform(orientation, deviceToWorldSpace.TransformPosition(devicePosition));
			return true;
		}
	}

	WorldTransform = FTransform::Identity;
	return false;
}