#include "PhysicalObjectTrackingReferencePoint.h"

#include "PhysicalObjectTrackingUtility.h"

void UPhysicalObjectTrackingReferencePoint::SetOriginTransform(const FQuat& NeutralRotation,
	const FVector& NeutralPosition)
{
	check(!IsRunningGame());
	CalibrationSteamVRToOriginOffset = NeutralPosition;
	CalibrationSteamVRToOriginRotation = NeutralRotation.Inverse();
}

void UPhysicalObjectTrackingReferencePoint::SetBaseStationOffsets(const TMap<int32, FTransform>& Offsets)
{
	BaseStationOffsetsToOrigin.Empty();
	for (const auto& baseStation : Offsets)
	{
		FString baseStationSerialId;
		if (FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(baseStation.Key, baseStationSerialId))
		{
			FTransform baseStationTransform = baseStation.Value;
			FQuat rotation = baseStation.Value.GetRotation() * CalibrationSteamVRToOriginRotation;
			FVector offset = (CalibrationSteamVRToOriginRotation * baseStation.Value.GetLocation()) - CalibrationSteamVRToOriginOffset;

			BaseStationOffsetsToOrigin.Add(baseStationSerialId, FTransform(rotation, offset));
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

const FQuat& UPhysicalObjectTrackingReferencePoint::GetNeutralRotationInverse() const
{
	return CalibrationSteamVRToOriginRotation;
}

const FVector& UPhysicalObjectTrackingReferencePoint::GetNeutralOffset() const
{
	return CalibrationSteamVRToOriginOffset;
}

FTransform UPhysicalObjectTrackingReferencePoint::ApplyTransformation(const FVector& TrackedPosition,
	const FQuat& TrackedRotation) const
{
	static const FMatrix deviceToWorldSpace = 
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
	return FTransform(orientation, position);
}

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationWorldTransform(const FString& BaseStationSerialId, FTransform& Result) const
{
	if (const FTransform* calibration = BaseStationOffsetsToOrigin.Find(BaseStationSerialId))
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
