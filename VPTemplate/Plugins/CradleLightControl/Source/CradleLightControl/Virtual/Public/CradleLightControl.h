// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "LightControlTool.h"

#include "GelPaletteWidget.h"

#include "IDetailCustomization.h"
#include "Chaos/AABB.h"

class FCradleLightControlModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	static bool OpenFileDialog(FString Title, void*
                        NativeWindowHandle, FString DefaultPath, uint32 Flags, FString FileTypeList, TArray<FString>& OutFilenames);
	static bool SaveFileDialog(FString Title, void*
                        NativeWindowHandle, FString DefaultPath, uint32 Flags, FString FileTypeList, TArray<FString>& OutFilenames);

	void OpenGelPalette(FGelPaletteSelectionCallback SelectionCallback);

	void RegisterTabSpawner();
	void RegisterDMXTabSpawner();



	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<SDockTab> LightTab;
	TSharedPtr<SDockTab> DMXTab;

	TSharedPtr<SLightControlTool> LightControl;
	TSharedPtr<class SDMXControlTool> DMXControl;

	TSharedPtr<SGelPaletteWidget> GelPalette;
	TSharedPtr<SWindow> GelPaletteWindow;


private:

};
