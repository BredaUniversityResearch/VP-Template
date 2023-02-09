#pragma once
#include "TrackerTransformHistory.h"

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
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingReferencePoint: public UDataAsset, public FTickableGameObject
{
	GENERATED_BODY()

public:

	UPhysicalObjectTrackingReferencePoint(const FObjectInitializer& ObjectInitializer);

	void Tick(float DeltaTime) override;

	bool IsTickableInEditor() const override;

	FORCEINLINE TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UPhysicalObjectTrackingReferencePoint, STATGROUP_Default); }

	void SetTrackerCalibrationTransform(const FTransform& Transform);
	void SetBaseStationCalibrationTransform(
		const FString& BaseStationSerialId, 
		const FTransform& Transform, 
		const FColor& Color, 
		bool StaticCalibration);
	void ResetBaseStationOffsets();

	//Get the tracker transform in SteamVR space at calibration time that is stored in the reference point.
	const FTransform& GetTrackerCalibrationTransform() const;
	//Get the base station transforms in SteamVR space at calibration time,
	//mapped to the serial ids of the base stations, that are stored in the the reference point
	const TMap<FString, FTransform>& GetBaseStationCalibrationTransforms() const;

	//Used for getting base station transformations
	//(legacy function for tracking trackers, currently only used for debug visualization and as fallback function).
	FTransform ApplyTransformation(const FVector& TrackedPosition, const FQuat& TrackedRotation) const;

	//Get the tracker's reference-space transform. Expects raw SteamVR space input.
	//(tracker rotation fix can be done using FPhysicalObjectTrackingUtility::FixTrackerTransform())
	FTransform GetTrackerReferenceSpaceTransform(const FTransform& TrackerCurrentTransform) const;
	//Get the base station's reference-space transform if the serial id matches a stored serial id. Expects raw SteamVR space input.
	//(Only used for drawing debug visualizations, uses string lookups which are slow.)
	bool GetBaseStationReferenceSpaceTransform(const FString& BaseStationSerialId, FTransform& WorldTransform) const;

	//Update the run time data if needed, which includes:
	//- Mapping the base station calibration transforms which are mapped to serial ids as strings
	//to a map which uses device ids to refer to base stations. (Removes slow string lookups)
	void UpdateRuntimeDataIfNeeded();

private:

	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;
	virtual void PostReinitProperties() override;

	bool HasMappedAllBaseStations() const;
	bool MapBaseStationIds();

	void UpdateAveragedBaseStationOffset();
	
	/* Flips up/down rotation */
	UPROPERTY(EditAnywhere, Category= "PhysicalObjectTrackingReferencePoint|Rotation")
	bool InvertPitchRotation{ false };
	/* Flips left/right rotation */
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|Rotation")
	bool InvertYawRotation{false};
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|Rotation")
	bool InvertRollRotation{ false };

	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FTransform TrackerCalibrationTransform;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	TMap<FString, FTransform> BaseStationCalibrationTransforms;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	TMap<FString, FBaseStationCalibrationInfo> BaseStationCalibrationInfo;
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint", meta=(ClampMin=1))
	int32 BaseStationOffsetHistorySize;
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint", meta=(ClampMin=0.001))
	float BaseStationOffsetUpdatesPerSecond {4.f};

	//Runtime data
	
	TMap<int32, FTransform> BaseStationIdToCalibrationTransform;

	FTrackerTransformHistory AveragedBaseStationOffsetHistory;
	FTransform AveragedBaseStationOffsetCached;
	bool AveragedBaseStationOffsetCachedValid{ false };

	float UpdateBaseStationOffsetsDeltaTimeAccumulator{ 0.f };

};

