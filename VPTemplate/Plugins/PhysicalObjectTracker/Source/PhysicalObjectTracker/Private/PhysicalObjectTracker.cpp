#include "PhysicalObjectTracker.h"

#include "ContentBrowserModule.h"

#include "IXRTrackingSystem.h"

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

int32 FPhysicalObjectTracker::GetDeviceIdFromSerialId(FString SerialId)
{
	if (GEngine && GEngine->XRSystem && !SerialId.IsEmpty())
	{
		auto XR = GEngine->XRSystem;
	
		TArray<int32> DeviceIds;
		XR->EnumerateTrackedDevices(DeviceIds);

		for (auto DeviceId : DeviceIds)
		{
			auto DeviceSerial = XR->GetTrackedDevicePropertySerialNumber(DeviceId);
			if (DeviceSerial == SerialId)
			{
				return DeviceId;
			}

		}
	}

	return -1;
		
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhysicalObjectTracker, PhysicalObjectTracker)