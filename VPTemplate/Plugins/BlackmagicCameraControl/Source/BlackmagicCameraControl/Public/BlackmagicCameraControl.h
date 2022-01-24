#pragma once

#include "CoreMinimal.h"
#include "BMCCService.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBlackmagicCameraControl, Log, All);

class FBlackmagicCameraControl : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TUniquePtr<FBMCCService> ControlService;
};