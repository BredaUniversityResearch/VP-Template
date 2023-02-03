// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTCPMessagingServer;

class FDataLinkModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	TUniquePtr<FTCPMessagingServer> MessagingServer;

};

// - Application or Unreal Engine has event that happens when a take starts recording. (How do we detect when a take starts? Can it be done through unreal? Is it already being done in the data wrangler?)
// - When a take starts: A message should be sent to the DataLink to start sending tracking data.
// (Should it be possible to specify which trackers to record or should it just take all the active UPhysicalObjectTrackingComponents in the scene and send their data.)
// (If data for multiple tracker is being sent, how do we distinguish between different trackers? Possible solution is the SerialId DataAsset name, which would allow for changing trackers if battery dies but keeping the name consistent in takes.)
// - When the message to start sending tracking data is received the DataLink plugin should start sending tracker data for each time code.
// (How do we get the time codes? If the time code provider is set up in unreal, we might be able to use events from this.)
