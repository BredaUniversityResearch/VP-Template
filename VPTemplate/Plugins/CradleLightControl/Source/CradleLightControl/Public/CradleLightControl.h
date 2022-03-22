// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
//#include "LightControlTool.h"

//#include "GelPaletteWidget.h"

#include "Chaos/AABB.h"

#include "LightRuntimeNetwork.h"


class UToolData;
class ILightRuntimeNetwork;
class ILightEditorNetwork;

DECLARE_LOG_CATEGORY_EXTERN(LogCradleLightControl, Log, All)

class CRADLELIGHTCONTROL_API FCradleLightControlModule : public IModuleInterface
{
public:

	~FCradleLightControlModule();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Shorthand to get the module instance if it is loaded
	static FCradleLightControlModule& Get();

	static ILightRuntimeNetwork& GetRuntimeNetworkInterface() { return *Get().RuntimeNetworkInterface; }
	static ILightEditorNetwork& GetEditorNetworkInterface() { return *Get().EditorNetworkInterface; }
	
	UToolData* GetVirtualLightToolData();
	UToolData* GetDMXLightToolData();

	

	ILightEditorNetwork* EditorNetworkInterface;
private:

	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues);
	void OnWorldCleanup(UWorld*, bool, bool);

	void OnActorSpawned(AActor* SpawnedActor);
	void VerifyVirtualLightsData();

	UToolData* VirtualLightToolData;
	UToolData* DMXLightToolData;

	TUniquePtr<ILightRuntimeNetwork> RuntimeNetworkInterface;

	FDelegateHandle ActorSpawnedDelegate;
	FTimerHandle VirtualLightVerificationTimerHandle;
};
