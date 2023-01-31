#pragma once

#include "PhysicalObjectTrackerSerialId.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnSerialIdChanged);

UCLASS()
class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackerSerialId : public UDataAsset
{
    GENERATED_BODY()
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void SetSerialId(const FString& InSerialId);
	const FString& GetSerialId() const;

	FOnSerialIdChanged OnSerialIdChanged;

private:

	UPROPERTY(EditAnywhere, Category = "PhysicalObjectTrackingComponent")
	FString SerialId;

};