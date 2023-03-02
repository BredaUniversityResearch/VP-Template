#pragma once
#include "TrackerTransformHistory.h"

#include "PhysicalObjectTrackingReferencePoint.generated.h"

USTRUCT(Category = "PhysicalObjectTrackingReferencePoint")
struct FBaseStationCalibrationInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	FTransform Transformation;
	UPROPERTY(VisibleAnywhere, Category = "PhysicalObjectTrackingReferencePoint")
	bool StaticallyCalibrated;
	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingReferencePoint")
	FColor Color;
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
	void SetBaseStationCalibrationInfo(
		const FString& BaseStationSerialId, 
		const FTransform& Transform, 
		const FColor& Color, 
		bool StaticCalibration);

	//Get the tracker transform in SteamVR space at calibration time that is stored in the reference point.
	const FTransform& GetTrackerCalibrationTransform() const;
	//Get the base station's reference-space transform if the serial id matches a stored serial id. Expects raw SteamVR space input.
	//(Only used for drawing debug visualizations, uses string lookups which are slow.)
	bool GetBaseStationCalibrationTransform(const FString& BaseStationSerialId, FTransform& OutReferenceSpaceTransform, FTransform& OutRawTransform) const;
	bool GetBaseStationColor(const FString& BaseStationSerialId, FColor& OutColor) const;

	//Used for getting base station transformations
	//(legacy function for tracking trackers, currently only used for debug visualization and as fallback function).
	FTransform ApplyTransformation(const FTransform& TrackedTransform) const;
	FTransform ApplyTransformation(const FVector& TrackedPosition, const FQuat& TrackedRotation) const;

	//Get the tracker's reference-space transform. Expects raw SteamVR space input.
	FTransform GetTrackerReferenceSpaceTransform(const FTransform& TrackerCurrentTransform) const;

	//Update the run time data if needed, which includes:
	//- Mapping the base station calibration transforms which are mapped to serial ids as strings
	//to a map which uses device ids to refer to base stations. (Removes slow string lookups)
	void UpdateRuntimeDataIfNeeded();

	int32 GetMinBaseStationsCalibrated() const;
	int32 GetMinBaseStationsCalibratedStatically() const;

private:

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;
	virtual void PostReinitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

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

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|BaseStations", meta = (ClampMin = 1))
	int32 MinNumBaseStationsCalibrated {6};
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|BaseStations", meta = (ClampMin = 1))
	int32 MinNumBaseStationsCalibratedStatically {2};
	/*UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|BaseStations", meta = (ClampMin = 1))
	int32 BaseStationOffsetHistorySize {100};*/
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingReferencePoint|BaseStations", meta = (ClampMin = 0.001))
	float BaseStationOffsetUpdatesPerSecond{ 25.f };
	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingReferencePoint|BaseStations")
	TMap<FString, FBaseStationCalibrationInfo> BaseStationCalibrationInfo;

	//Runtime data
	//Maps the calibration transforms of the base stations to Device Ids instead of Serial Ids.
	//(Serial Ids are consistent between sessions but Device Ids not necesarilly)
	TMap<int32, FTransform> BaseStationIdToCalibrationTransforms;

	TMap<int32, FBaseStationCalibrationInfo> BaseStationIdToInfo;

	//TODO: handle resizing of samples if MinNumBaseStationsCalibrated changes.
	FTrackerTransformHistory BaseStationOffsetSamples;
	bool AveragedBaseStationOffsetCachedValid{ false };

	UPROPERTY(VisibleAnywhere, Transient, Category = "PhysicalObjectTrackingReferencePoint|RunTime")
	FTransform AveragedBaseStationOffsetCached;

	float UpdateBaseStationOffsetsDeltaTimeAccumulator{ 0.f };

};

