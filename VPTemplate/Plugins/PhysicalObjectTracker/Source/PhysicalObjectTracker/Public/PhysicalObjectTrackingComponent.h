#pragma once
#include "TrackerTransformHistory.h"


#include "PhysicalObjectTrackingComponent.generated.h"

class UPhysicalObjectTrackingFilterSettings;
class UPhysicalObjectTrackingReferencePoint;
UCLASS(ClassGroup = (VirtualProduction), meta = (BlueprintSpawnableComponent))
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingComponent: public UActorComponent
{
	GENERATED_BODY()
public:
	explicit UPhysicalObjectTrackingComponent(const FObjectInitializer& ObjectInitializer);
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(CallInEditor, Category = "PhysicalObjectTrackingComponent")
	void SelectTracker();

	UFUNCTION(CallInEditor, Category = "PhysicalObjectTrackingComponent")
	void RefreshDeviceId();

	const FTransform* GetWorldReferencePoint() const;
	const UPhysicalObjectTrackingReferencePoint* GetTrackingReferencePoint() const;

	UPROPERTY(Transient, VisibleAnywhere, Category = "PhysicalObjectTrackingComponent")
	int32 CurrentTargetDeviceId{-1};

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent", meta=(DeviceSerialId))
	FString SerialId;

private:
	void DebugCheckIfTrackingTargetExists() const;
	void OnFilterSettingsChangedCallback();

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingReferencePoint* TrackingSpaceReference{nullptr};
	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent")
	AActor* WorldReferencePoint{nullptr};
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingFilterSettings* FilterSettings;
	FDelegateHandle FilterSettingsChangedHandle;

	UPROPERTY(Transient)
	float DeviceIdAcquireTimer;
	FTrackerTransformHistory m_TransformHistory;

	UPROPERTY()
	float DeviceReacquireInterval {0.5f};
};