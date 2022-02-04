#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicalObjectTracker, Log, All);

DECLARE_EVENT_OneParam(FPhysicalObjectTracker, FDeviceDetectionStarted, class UPhysicalObjectTrackingComponent*)

class UPhysicalObjectTrackingReferencePoint;
class FDetectTrackerShakeTask;
class FPhysicalObjectTracker : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void DebugDrawTrackingReferenceLocations(const UPhysicalObjectTrackingReferencePoint* ReferencePoint);
	
	FDeviceDetectionStarted DeviceDetectionEvent;

};