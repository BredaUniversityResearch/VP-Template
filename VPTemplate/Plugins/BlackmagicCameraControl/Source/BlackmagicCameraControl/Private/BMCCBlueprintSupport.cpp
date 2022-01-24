#include "BMCCBlueprintSupport.h"

#include "BMCCService.h"

#include "BlackmagicCameraControl.h"

UBMCCDispatcher* UBMCCBlueprintSupport::GetBlackmagicCameraControlDispatcher()
{
	IModularFeatures& features = IModularFeatures::Get();
	if (features.IsModularFeatureAvailable(FBMCCService::GetModularFeatureName()))
	{
		FBMCCService& service = features.GetModularFeature<FBMCCService>(FBMCCService::GetModularFeatureName());
		return service.GetDefaultDispatcher();
	}
	else
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("BlackMagicCameraControlService is not available as feature"));
		return nullptr;
	}
}

float UBMCCBlueprintSupport::Conv_Fixed16ToFloat(FBMCCFixed16 Value)
{
	return Value.AsFloat();
}
