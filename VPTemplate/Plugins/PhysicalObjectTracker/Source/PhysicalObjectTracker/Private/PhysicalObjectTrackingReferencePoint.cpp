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
