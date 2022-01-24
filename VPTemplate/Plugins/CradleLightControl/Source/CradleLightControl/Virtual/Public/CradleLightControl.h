// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
//#include "LightControlTool.h"

//#include "GelPaletteWidget.h"

#include "IDetailCustomization.h"
#include "Chaos/AABB.h"

class UToolData;

DECLARE_LOG_CATEGORY_EXTERN(LogCradleLightControl, Log, All)

class CRADLELIGHTCONTROL_API FCradleLightControlModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// Shorthand to get the module instance if it is loaded
	static FCradleLightControlModule& Get();

	UToolData* GetVirtualLightToolData();
	UToolData* GetDMXLightToolData();


private:

	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues);
	void OnWorldCleanup(UWorld*, bool, bool);

	UToolData* VirtualLightToolData;
	UToolData* DMXLightToolData;
};
