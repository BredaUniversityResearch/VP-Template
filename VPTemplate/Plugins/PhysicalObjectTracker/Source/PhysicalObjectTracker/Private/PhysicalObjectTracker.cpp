#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponentRegistry.h"

#define LOCTEXT_NAMESPACE "FPhysicalObjectTracker"

DEFINE_LOG_CATEGORY(LogPhysicalObjectTracker);

void FPhysicalObjectTracker::StartupModule()
{
	ObjectTrackingComponents = MakeShared<FPhysicalObjectTrackingComponentRegistry>();
}

void FPhysicalObjectTracker::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhysicalObjectTracker, PhysicalObjectTracker)