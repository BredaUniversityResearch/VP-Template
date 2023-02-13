#include "PhysicalObjectTracker.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTracker"

DEFINE_LOG_CATEGORY(LogPhysicalObjectTracker);

void FPhysicalObjectTracker::StartupModule()
{
}

void FPhysicalObjectTracker::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FPhysicalObjectTracker::AddObjectTrackingComponent(TObjectPtr<UPhysicalObjectTrackingComponent> Component)
{
	CurrentObjectTrackers.Add(Component);
	OnTrackingComponentRegistered.Broadcast(Component);
}

void FPhysicalObjectTracker::RemoveObjectTrackingComponent(TObjectPtr<UPhysicalObjectTrackingComponent> Component)
{
	CurrentObjectTrackers.Remove(Component);
	OnTrackingComponentUnregistered.Broadcast(Component);
}

const TSet<TObjectPtr<UPhysicalObjectTrackingComponent>>& FPhysicalObjectTracker::GetCurrentObjectTrackers() const
{
	return CurrentObjectTrackers;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhysicalObjectTracker, PhysicalObjectTracker)