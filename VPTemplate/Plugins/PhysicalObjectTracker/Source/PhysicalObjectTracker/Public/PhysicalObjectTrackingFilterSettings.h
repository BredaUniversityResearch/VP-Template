#pragma once

#include "PhysicalObjectTrackingFilterSettings.generated.h"

UCLASS()
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingFilterSettings : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	int32 TargetSampleCount{16};
	UPROPERTY(EditDefaultsOnly)
	float FilterExponent{ 2.0f };
	UPROPERTY(EditDefaultsOnly) /*Average cm per sample frame.*/
	float MinExpectedLinearVelocity{ 0.001f };
	UPROPERTY(EditDefaultsOnly) /*Average cm per sample frame.*/
	float MaxExpectedLinearVelocity{ 2.0f };
	UPROPERTY(EditDefaultsOnly) /*Average Degrees per sample frame.*/
	float MinExpectedRotationalVelocity{ 0.01f };
	UPROPERTY(EditDefaultsOnly) /*Average Degrees per sample frame.*/
	float MaxExpectedRotationalVelocity{ 2.0f };
};

