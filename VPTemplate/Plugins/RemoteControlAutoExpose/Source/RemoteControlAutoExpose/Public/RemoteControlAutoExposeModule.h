#pragma once

#include "CoreMinimal.h"

class FRemoteControlAssetActionExtension;

class REMOTECONTROLAUTOEXPOSE_API FRemoteControlAutoExposeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TUniquePtr<FRemoteControlAssetActionExtension> m_Extender;
};
