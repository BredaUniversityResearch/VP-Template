#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicalObjectTracker, Log, All);

DECLARE_EVENT_OneParam(FPhysicalObjectTracker, FDeviceDetectionStarted, class UPhysicalObjectTrackingComponent*)

class FDetectTrackerShakeTask;
class FPhysicalObjectTracker : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static int32 GetDeviceIdFromSerialId(FString SerialId);
	
	FDeviceDetectionStarted DeviceDetectionEvent;

};