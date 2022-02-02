#pragma once


#include "PhysicalObjectTrackingComponent.generated.h"

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

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	

	UFUNCTION(CallInEditor, Category = "PhysicalObjectTrackingComponent")
	void SelectTracker();

	UFUNCTION(CallInEditor, Category = "PhysicalObjectTrackingComponent")
	void RefreshDeviceId();

	UPROPERTY(Transient, VisibleAnywhere, Category = "PhysicalObjectTrackingComponent")
	int32 CurrentTargetDeviceId{-1};

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent", meta=(DeviceSerialId))
	FString SerialId;

private:
	void DebugCheckIfTrackingTargetExists() const;
	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	UPhysicalObjectTrackingReferencePoint* TrackingSpaceReference{nullptr};
	UPROPERTY(EditInstanceOnly, Category = "PhysicalObjectTrackingComponent")
	AActor* WorldReferencePoint{nullptr};

	UPROPERTY(Transient)
	float DeviceIdAcquireTimer;

	UPROPERTY()
	float DeviceReacquireInterval {0.5f};
};