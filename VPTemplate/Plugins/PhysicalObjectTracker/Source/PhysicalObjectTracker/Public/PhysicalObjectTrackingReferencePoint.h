#pragma once

#include "PhysicalObjectTrackingReferencePoint.generated.h"

USTRUCT(Category = "PhysicalObjectTrackingReferencePoint")
struct FBaseStationCalibrationInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	bool StaticallyCalibrated;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FColor Color;
};

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

	void SetTrackerCalibrationTransform(const FTransform& Transform);
	void SetBaseStationCalibrationTransform(
		const FString& BaseStationSerialId, 
		const FTransform& Transform, 
		const FColor& Color, 
		bool StaticCalibration);
	void ResetBaseStationOffsets();

	const FTransform& GetTrackerCalibrationTransform() const;
	const TMap<FString, FTransform>& GetBaseStationCalibrationTransforms() const;

	FTransform ApplyTransformation(const FVector& TrackedPosition, const FQuat& TrackedRotation) const;

	FTransform GetTrackerWorldTransform(const FTransform& TrackerCurrentTransform) const;
	//Only used for drawing debug visualizations, uses string lookups which are slow.
	bool GetBaseStationWorldTransform(const FString& BaseStationSerialId, FTransform& WorldTransform) const;

private:

	virtual void PostInitProperties() override;
	//virtual void PostLoad() override;
	void MapBaseStationIds();

	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FTransform TrackerCalibrationTransform;
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
	TMap<FString, FTransform> BaseStationCalibrationTransforms;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	TMap<FString, FBaseStationCalibrationInfo> BaseStationCalibrationInfo;

	UPROPERTY(Transient)
	TMap<int32, FTransform> BaseStationIdToCalibrationTransform;

};

