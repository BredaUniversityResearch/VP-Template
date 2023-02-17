#pragma once

#include "CoreMinimal.h"
#include "PhysicalObjectTrackingComponentRegistry.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicalObjectTracker, Log, All);

class UPhysicalObjectTrackingReferencePoint;
class FDetectTrackerShakeTask;
class FPhysicalObjectTracker : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FPhysicalObjectTrackingComponentRegistry ObjectTrackingComponents;

};