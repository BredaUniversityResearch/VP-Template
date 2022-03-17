// Copyright Epic Games, Inc. All Rights Reserved.

#include "CradleLightControl.h"

#define LOCTEXT_NAMESPACE "FCradleLightControlModule"

DEFINE_LOG_CATEGORY(LogCradleLightControl)

void FCradleLightControlModule::StartupModule()
{
	m_Server = MakeUnique<FLightControlWebSocketServer>();
	if (!m_Server->Start(32800))
	{
		UE_LOG(LogCradleLightControl, Error, TEXT("Failed to start WebSocket server"));
	}
}

void FCradleLightControlModule::ShutdownModule()
{
	m_Server.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCradleLightControlModule, CradleLightControl)