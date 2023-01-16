#pragma once

#include "PhysicalObjectTrackingReferencePoint.generated.h"

USTRUCT(Category = "PhysicalObjectTrackingReferencePoint")
struct FBaseStationOffset
{
    GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FVector Position;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FQuat Rotation;

};

UCLASS(BlueprintType)
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingReferencePoint: public UDataAsset
{
	GENERATED_BODY()

public:
	void SetNeutralTransform(const FQuat& NeutralRotation, const FVector& NeutralPosition);
	void SetInitialBaseStationOffset(int32 BaseStationId, const FQuat& RotationOffset, const FVector& PositionOffset);

	const FQuat& GetNeutralRotationInverse() const;
	const FVector& GetNeutralOffset() const;
	FTransform ApplyTransformation(const FVector& TrackedPosition, const FQuat& TrackedRotation) const;

	void GetBaseStationIds(TArray<int32>& BaseStationIds) const;
	FTransform CalcTransformationFromBaseStations(const TMap<int32, FBaseStationOffset>& BaseStationOffsets);
	

private:
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FQuat NeutralRotationInverse;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FVector NeutralOffset;
	/* Flips up/down rotation */
	UPROPERTY(EditAnywhere, Category= "PhysicalObjectTrackingReferencePoint|Rotation")
	bool InvertPitchRotation{ false };
	/* Flips left/right rotation */
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|Rotation")
	bool InvertYawRotation{false};
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|Rotation")
	bool InvertRollRotation{ false };

	TMap<int32, FBaseStationOffset> BaseStationOffsets;
};

