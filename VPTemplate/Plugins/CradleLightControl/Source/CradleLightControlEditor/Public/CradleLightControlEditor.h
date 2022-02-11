// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "LightControlTool.h"

#include "GelPaletteWidget.h"

#include "IDetailCustomization.h"
#include "Chaos/AABB.h"


UENUM()
enum EIconType
{
	GeneralLightOff = 0,
	GeneralLightOn,
	GeneralLightUndetermined,
	SkyLightOff,
	SkyLightOn,
	SkyLightUndetermined,
	SpotLightOff,
	SpotLightOn,
	SpotLightUndetermined,
	DirectionalLightOff,
	DirectionalLightOn,
	DirectionalLightUndetermined,
	PointLightOff,
	PointLightOn,
	PointLightUndetermined,
	FolderClosed,
	FolderOpened
};

class UItemHandle;
class ILightEditorNetwork;
// About module: Editor-only module, contains the code for the UI
// Separated from the core module of the plugin because it uses the editor's icons, which are unavailable in standalone 

class FCradleLightControlEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void AddToolBarButton(FToolBarBuilder& ToolbarBuilder);
	void OnToolBarButtonClicked();

	void OnEngineExit();

	static bool OpenFileDialog(FString Title, void*
		NativeWindowHandle, FString DefaultPath, uint32 Flags, FString FileTypeList, TArray<FString>& OutFilenames);
	static bool SaveFileDialog(FString Title, void*
		NativeWindowHandle, FString DefaultPath, uint32 Flags, FString FileTypeList, TArray<FString>& OutFilenames);

	static FCradleLightControlEditorModule& Get();

	static ILightEditorNetwork& GetLightPropertyChangeSpeaker() { return *Get().EditorNetworkInterface; }

	static TArray<UBaseLight*> GetLightsFromHandles(TArray<UItemHandle*> Handles);

	void OpenGelPalette(FGelPaletteSelectionCallback SelectionCallback);
	void CloseGelPalette();

	void RegisterTabSpawner();
	void RegisterDMXTabSpawner();

	// Generates the widget for the given item handle. Necessary because icons are in the editor module rather than the core module.
	// This widget is used only in the tree hierarchy
	//void GenerateItemHandleWidget(UItemHandle* ItemHandle);

	void GenerateIcons();
	// Meant to be used with only the light icon types.
	FCheckBoxStyle MakeCheckboxStyleForType(uint8 IconType);
	FSlateBrush& GetIcon(EIconType Icon);


	TSharedPtr<FUICommandList> CommandList;

	TSharedPtr<SDockTab> LightTab;
	TSharedPtr<SDockTab> DMXTab;

	TSharedPtr<SLightControlTool> VirtualLightControl;
	TSharedPtr<class SDMXControlTool> DMXControl;

	TSharedPtr<SGelPaletteWidget> GelPalette;
	TSharedPtr<SWindow> GelPaletteWindow;


	TMap<TEnumAsByte<EIconType>, FSlateBrush> Icons;
private:

	TUniquePtr<ILightEditorNetwork> EditorNetworkInterface;
};
