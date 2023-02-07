#pragma once

#include "CoreMinimal.h"

class UPhysicalObjectTrackingComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhysicalObjectTrackingComponentRegistered, TSharedRef<UPhysicalObjectTrackingComponent>)

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicalObjectTracker, Log, All);

class UPhysicalObjectTrackingReferencePoint;
class FDetectTrackerShakeTask;
class FPhysicalObjectTracker : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FOnPhysicalObjectTrackingComponentRegistered OnTrackingComponentRegistered;
};