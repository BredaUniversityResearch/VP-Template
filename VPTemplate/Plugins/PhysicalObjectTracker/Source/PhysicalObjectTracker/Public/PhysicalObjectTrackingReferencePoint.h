#pragma once

#include "PhysicalObjectTrackingReferencePoint.generated.h"

UCLASS(BlueprintType)
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingReferencePoint: public UDataAsset
{
	GENERATED_BODY()
public:
	void SetNeutralTransform(const FQuat& NeutralRotation, const FVector& NeutralPosition);

	const FQuat& GetNeutralRotationInverse() const;
	const FVector& GetNeutralOffset() const;
	FTransform ApplyTransformation(const FVector& TrackedPosition, const FQuat& TrackedRotation) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FQuat NeutralRotationInverse;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FVector NeutralOffset;
};

