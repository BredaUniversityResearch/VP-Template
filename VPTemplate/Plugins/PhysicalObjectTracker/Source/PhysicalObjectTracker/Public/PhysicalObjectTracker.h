#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicalObjectTracker, Log, All);

class FPhysicalObjectTrackingComponentRegistry;
class UPhysicalObjectTrackingReferencePoint;
class FDetectTrackerShakeTask;

class FPhysicalObjectTracker : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FPhysicalObjectTrackingComponentRegistry> ObjectTrackingComponents;

};