#pragma once
#include "TrackerTransformHistory.h"
#include "TrackerTransformUpdates.h"

#include "PhysicalObjectTrackingComponent.generated.h"

class UPhysicalObjectTrackingFilterSettings;
class UPhysicalObjectTrackingReferencePoint;
class UPhysicalObjectTrackerSerialId;
UCLASS(ClassGroup = (VirtualProduction), meta = (BlueprintSpawnableComponent))
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingComponent: public UActorComponent
{
	GENERATED_BODY()
public:
	explicit UPhysicalObjectTrackingComponent(const FObjectInitializer& ObjectInitializer);
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(CallInEditor, Category = "PhysicalObjectTrackingComponent")
	void RefreshDeviceId();

	const FTransform* GetWorldReferencePoint() const;
	const UPhysicalObjectTrackingReferencePoint* GetTrackingReferencePoint() const;

	UPROPERTY(Transient, VisibleAnywhere, Category = "PhysicalObjectTrackingComponent")
	int32 CurrentTargetDeviceId {-1};

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	TObjectPtr<UPhysicalObjectTrackerSerialId> TrackerSerialIdAsset;

	FTrackerTransformUpdates TransformUpdates;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing")
	bool DisableDebugDrawing = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations")
	bool ShowBaseStationsCalibration = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations")
	bool ShowBaseStationsCalibrationRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations")
	bool ShowBaseStationsCurrent = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations")
	bool ShowBaseStationsCurrentRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations")
	bool ShowBaseStationsFixed = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations")
	bool ShowBaseStationsFixedRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker")
	bool ShowTrackerCalibration = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker")
	bool ShowTrackerCalibrationRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker")
	bool ShowTrackerCurrent = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker")
	bool ShowTrackerCurrentRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker")
	bool ShowTrackerFixed = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker")
	bool ShowTrackerFixedRaw = false;

#endif

private:
	void DebugCheckIfTrackingTargetExists() const;
	void OnFilterSettingsChangedCallback();
	void OnTrackerSerialIdChangedCallback();
	void ExtractTransformationTargetComponentReferenceIfValid();

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingReferencePoint* TrackingSpaceReference{nullptr};
	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent")
	AActor* WorldReferencePoint{nullptr};
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingFilterSettings* FilterSettings;

	FDelegateHandle FilterSettingsChangedHandle;
	FDelegateHandle SerialIdChangedHandle;
	
	UPROPERTY(EditAnyWhere, Category = "PhysicalObjectTrackingComponent",
		meta = (ToolTip = "Configure a component to move according to the tracker, otherwise moves this component's actor."))
	bool HasTransformationTargetComponent;
	UPROPERTY(EditAnyWhere, Category = "PhysicalObjectTrackingComponent",
		meta = (ToolTip="Leave the Actor field empty to specify a component on this actor.", EditCondition = "HasTransformationTargetComponent", EditConditionHides))
	FComponentReference TransformationTargetComponentReference;
	UPROPERTY(Transient, Category = "PhysicalObjectTrackingComponent", VisibleInstanceOnly)
	TObjectPtr<USceneComponent> TransformationTargetComponent {nullptr};

	UPROPERTY(Transient)
	float DeviceIdAcquireTimer;
	UPROPERTY(Transient)
	float DeviceReacquireInterval{ 0.5f };

	FTrackerTransformHistory m_TransformHistory;
};