#include "BlackmagicCameraControl.h"

#define LOCTEXT_NAMESPACE "BlackMagicCameraControl"

DEFINE_LOG_CATEGORY(LogBlackmagicCameraControl);

void FBlackmagicCameraControl::StartupModule()
{
	ControlService = MakeUnique<FBMCCService>();
	IModularFeatures::Get().RegisterModularFeature(FBMCCService::GetModularFeatureName(), ControlService.Get());
}

void FBlackmagicCameraControl::ShutdownModule()
{
	IModularFeatures::Get().UnregisterModularFeature(FBMCCService::GetModularFeatureName(), ControlService.Get());
	ControlService.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlackmagicCameraControl, BlackMagicCameraControl)