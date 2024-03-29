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
	virtual void PostLoad() override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(CallInEditor, Category = "PhysicalObjectTrackingComponent")
	void RefreshDeviceId();

	const FTransform* GetWorldReferencePointTransform() const;
	const UPhysicalObjectTrackingReferencePoint* GetTrackingReferencePoint() const;

	UPROPERTY(Transient, VisibleAnywhere, Category = "PhysicalObjectTrackingComponent")
	int32 CurrentTargetDeviceId {-1};

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	TObjectPtr<UPhysicalObjectTrackerSerialId> TrackerSerialIdAsset;

	FTrackerTransformUpdates TransformUpdates;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing",
		meta=(ToolTip = "Completely disable the debug drawing of Base Stations and Trackers"))
	bool DisableDebugDrawing = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations",
		meta = (ToolTip = "Show the Base Stations at calibration time in \"ReferencePoint-Space\""))
	bool ShowBaseStationsCalibration = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations",
		meta = (ToolTip = "Show the Base Stations at calibration time in \"SteamVR-Space\""))
	bool ShowBaseStationsCalibrationRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations",
		meta = (ToolTip = "Show the Base Stations at current time in \"ReferencePoint-Space\""))
	bool ShowBaseStationsCurrent = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations",
		meta = (ToolTip = "Show the Base Stations at current time in \"SteamVR-Space\""))
	bool ShowBaseStationsCurrentRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations",
		meta = (ToolTip = "Show the Base Stations in \"ReferencePoint-Space\", adjusted to resolve the possible offset between current and calibration time"))
	bool ShowBaseStationsFixed = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|BaseStations",
		meta = (ToolTip = "Show the Base Stations in \"SteamVR-Space\", adjusted to resolve the possible offset between current and calibration time"))
	bool ShowBaseStationsFixedRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker",
		meta = (ToolTip = "Show the Tracker at calbiration time in \"ReferencePoint-Space\""))
	bool ShowTrackerCalibration = true;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker",
		meta = (ToolTip = "Show the Tracker at calbiration time in \"SteamVR-Space\""))
	bool ShowTrackerCalibrationRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker",
		meta = (ToolTip = "Show the Tracker at current time in \"ReferencePoint-Space\""))
	bool ShowTrackerCurrent = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker",
		meta = (ToolTip = "Show the Tracker at current time in \"SteamVR-Space\""))
	bool ShowTrackerCurrentRaw = false;

	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent|DebugDrawing|Tracker",
		meta = (ToolTip = "Show the Tracker at current time in \"ReferencePoint-Space\" with transformation calculated as an average off the offsets of the Base Station"))
	bool ShowTrackerFixed = true;

#endif

private:
	void DebugCheckIfTrackingTargetExists() const;
	void OnFilterSettingsChangedCallback();
	void OnTrackerSerialIdChangedCallback();
	USceneComponent* GetComponentToTransform() const;

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingReferencePoint* TrackingSpaceReference{nullptr};
	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent")
	AActor* WorldReferencePoint{nullptr};
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingFilterSettings* FilterSettings;

	FDelegateHandle FilterSettingsChangedHandle;
	FDelegateHandle SerialIdChangedHandle;
		
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent", 
		meta = (UseComponentPicker, AllowedClasses = "/Script/Engine.SceneComponent"))
	FComponentReference ComponentToTransform;
	UPROPERTY(Transient, Category = "PhysicalObjectTrackingComponent", VisibleInstanceOnly)
	TWeakObjectPtr<USceneComponent> TransformationTargetComponent {nullptr};

	UPROPERTY(Transient)
	float DeviceIdAcquireTimer;
	UPROPERTY(Transient)
	float DeviceReacquireInterval{ 0.5f };

	FTrackerTransformHistory m_TransformHistory;
};