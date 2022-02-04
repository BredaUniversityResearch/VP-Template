#pragma once

#include "PhysicalObjectTrackingFilterSettings.generated.h"

UCLASS()
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingFilterSettings : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	int32 TargetSampleCount{20};
	UPROPERTY(EditDefaultsOnly)
	float MinExpectedVelocity{ 5.0f };
	UPROPERTY(EditDefaultsOnly)
	float MaxExpectedVelocity{ 50.0f };
	UPROPERTY(EditDefaultsOnly)
	float FilterExponent{ 2.0f };
};

