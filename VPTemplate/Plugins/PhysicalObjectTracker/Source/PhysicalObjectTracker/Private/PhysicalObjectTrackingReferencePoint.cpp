#include "PhysicalObjectTrackingReferencePoint.h"

#include "SteamVRFunctionLibrary.h"

void UPhysicalObjectTrackingReferencePoint::SetNeutralTransform(const FQuat& NeutralRotation,
	const FVector& NeutralPosition)
{
	ensure(!IsRunningGame());
	NeutralOffset = NeutralPosition;
	NeutralRotationInverse = NeutralRotation.Inverse();
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
	FVector devicePosition = GetNeutralRotationInverse() * (TrackedPosition - GetNeutralOffset());
	FVector4 position = deviceToWorldSpace.TransformPosition(devicePosition);
	return FTransform(orientation, position);
}
