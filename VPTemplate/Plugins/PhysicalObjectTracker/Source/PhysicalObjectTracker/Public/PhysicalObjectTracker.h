#pragma once

#include "CoreMinimal.h"

class UPhysicalObjectTrackingComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhysicalObjectTrackingComponentRegistered, TObjectPtr<UPhysicalObjectTrackingComponent>)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhysicalObjectTrackingComponentUnregistered, TObjectPtr<UPhysicalObjectTrackingComponent>)

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicalObjectTracker, Log, All);

class UPhysicalObjectTrackingReferencePoint;
class FDetectTrackerShakeTask;
class FPhysicalObjectTracker : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void AddObjectTrackingComponent(TObjectPtr<UPhysicalObjectTrackingComponent> Component);
	void RemoveObjectTrackingComponent(TObjectPtr<UPhysicalObjectTrackingComponent> Component);

	const TSet<TObjectPtr<UPhysicalObjectTrackingComponent>>& GetCurrentObjectTrackers() const;

	FOnPhysicalObjectTrackingComponentRegistered OnTrackingComponentRegistered;
	FOnPhysicalObjectTrackingComponentUnregistered OnTrackingComponentUnregistered;

private:

	TSet<TObjectPtr<UPhysicalObjectTrackingComponent>> CurrentObjectTrackers;
};