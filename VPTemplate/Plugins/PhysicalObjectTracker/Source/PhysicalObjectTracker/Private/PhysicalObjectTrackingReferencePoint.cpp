#include "PhysicalObjectTrackingReferencePoint.h"

#include "SteamVRFunctionLibrary.h"

void UPhysicalObjectTrackingReferencePoint::SetNeutralTransform(const FQuat& NeutralRotation,
	const FVector& NeutralPosition)
{
	ensure(!IsRunningGame());
	NeutralOffset = NeutralPosition;
	NeutralRotationInverse = NeutralRotation.Inverse();
}

void UPhysicalObjectTrackingReferencePoint::SetInitialBaseStationOffset(
	int32 BaseStationId, 
	const FQuat& RotationOffset, 
	const FVector& PositionOffset)
{
	BaseStationOffsets.Add(BaseStationId, FBaseStationOffset{PositionOffset, RotationOffset});
}

const FQuat& UPhysicalObjectTrackingReferencePoint::GetNeutralRotationInverse() const
{
	return NeutralRotationInverse;
}

const FVector& UPhysicalObjectTrackingReferencePoint::GetNeutralOffset() const
{
	return NeutralOffset;
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

	FVector devicePosition = GetNeutralRotationInverse() * (TrackedPosition - GetNeutralOffset());
	FVector4 position = deviceToWorldSpace.TransformPosition(devicePosition);
	return FTransform(orientation, position);
}
void UPhysicalObjectTrackingReferencePoint::GetBaseStationIds(TArray<int32>& BaseStationIds) const
{
	BaseStationOffsets.GetKeys(BaseStationIds);
}

bool UPhysicalObjectTrackingReferencePoint::CalcTransformationFromBaseStations(
	const TMap<int32, FBaseStationOffset>& CurrentBaseStationOffsets,
	FTransform& CalculatedTransform)
{
	TArray<FBaseStationOffset> offsetDifferences;
	for(auto& currentOffset : CurrentBaseStationOffsets)
	{
		if (const FBaseStationOffset* initialOffset = BaseStationOffsets.Find(currentOffset.Key))
		{
			offsetDifferences.Add(
				FBaseStationOffset{
				initialOffset->Position - currentOffset.Value.Position,
				initialOffset->Rotation * currentOffset.Value.Rotation.Inverse()
				});
		}
	}

	if (offsetDifferences.Num() == 0) { return false; }

	CalculatedTransform = GetAveragedTransform(offsetDifferences);
	return true;
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
