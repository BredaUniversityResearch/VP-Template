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
	void SetBaseStationOffsetToOrigin(const FString& BaseStationSerialId, const FTransform& OffsetToOrigin);

	const FQuat& GetNeutralRotationInverse() const;
	const FVector& GetNeutralOffset() const;
	FTransform ApplyTransformation(const FVector& TrackedPosition, const FQuat& TrackedRotation) const;

private:

	static FTransform GetAveragedTransform(const TArray<FBaseStationOffset>& OffsetDifferences);

	//Base this off the offsets to the base stations.
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FQuat NeutralRotationInverse;
	//Base this off the offsets to the base stations.
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

	//The offset to the origin point for all base stations.
	//Stored with serial ids of the base stations as these don't change in-between different sessions, while device ids might.
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	TMap<FString, FTransform> BaseStationOffsetsToOrigin;

};

