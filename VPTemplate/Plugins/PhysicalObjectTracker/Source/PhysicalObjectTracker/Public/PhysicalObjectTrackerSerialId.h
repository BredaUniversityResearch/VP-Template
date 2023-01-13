#pragma once

#include "PhysicalObjectTrackerSerialId.generated.h"

UCLASS(BlueprintType)
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackerSerialId : public UDataAsset
{
    GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent", meta = (DeviceSerialId))
	FString SerialId;

};