#include "PhysicalObjectTrackingReferencePoint.h"

#include "SteamVRFunctionLibrary.h"

void UPhysicalObjectTrackingReferencePoint::SetNeutralTransform(const FQuat& NeutralRotation,
	const FVector& NeutralPosition)
{
	ensure(!IsRunningGame());
	NeutralOffset = NeutralPosition;
	NeutralRotationInverse = NeutralRotation.Inverse();
}

void UPhysicalObjectTrackingReferencePoint::SetBaseStationOffsetToOrigin(const FString& BaseStationSerialId, const FTransform& OffsetToOrigin)
{
	if(!BaseStationSerialId.IsEmpty())
	{
		BaseStationOffsetsToOrigin.Add(BaseStationSerialId, OffsetToOrigin);
	}
}

void UPhysicalObjectTrackingReferencePoint::ResetBaseStationOffsets()
{
	BaseStationOffsetsToOrigin.Empty();
}

const FQuat& UPhysicalObjectTrackingReferencePoint::GetNeutralRotationInverse() const
{
	return NeutralRotationInverse;
}

const FVector& UPhysicalObjectTrackingReferencePoint::GetNeutralOffset() const
{
	return NeutralOffset;
}

const TMap<FString, FTransform>& UPhysicalObjectTrackingReferencePoint::GetBaseStationOffsetsToOrigin() const
{
	return BaseStationOffsetsToOrigin;
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

bool UPhysicalObjectTrackingReferencePoint::GetBaseStationWorldTransform(const FString& BaseStationSerialId, FTransform& WorldTransform) const
{
	if(!BaseStationSerialId.IsEmpty())
	{
		if(const FTransform* baseStationOffset = BaseStationOffsetsToOrigin.Find(BaseStationSerialId))
		{
			static const FMatrix deviceToWorldSpace =
				FRotationMatrix::Make(FQuat(FVector::YAxisVector,
					FMath::DegreesToRadians(90))) * FScaleMatrix::Make(FVector(1.0f, -1.0f, -1.0f));

			FQuat orientation = baseStationOffset->GetRotation(); //The rotation is stored relative to the tracker's neutral rotation. 

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

			const FVector devicePosition = baseStationOffset->GetLocation(); //The position is stored relative to the neutral position and rotation.
			const FVector4 position = deviceToWorldSpace.TransformPosition(devicePosition);
			WorldTransform = FTransform(orientation, position);
			return true;
		}
	}

	WorldTransform = FTransform::Identity;
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
