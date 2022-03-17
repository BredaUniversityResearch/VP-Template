// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LightControlWebSocketServer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCradleLightControl, Log, All)

class CRADLELIGHTCONTROL_API FCradleLightControlModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:

	TUniquePtr<FLightControlWebSocketServer> m_Server;

};
