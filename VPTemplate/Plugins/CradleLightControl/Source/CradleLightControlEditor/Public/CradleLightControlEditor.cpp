// Copyright Epic Games, Inc. All Rights Reserved.

#include "CradleLightControlEditor.h"

#include "AssetToolsModule.h"
#include "LevelEditor.h"
#include "LightControlTool.h"
#include "DMXConfigAsset.h"
#include "DMXControlTool.h"

#include "BaseLight.h"

#include "ClassIconFinder.h"
#include "CradleLightControl.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "DirectLightEditorNetwork.h"
#include "DMXConfigAssetActions.h"

// About module: Editor-only module, contains the code for the UI
// Separated from the core module of the plugin because it uses the editor's icons, which are unavailable in standalone 

#define LOCTEXT_NAMESPACE "FCradleLightControlEditorModule"

DEFINE_LOG_CATEGORY(LogCradleLightControl)

void FCradleLightControlEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.NotifyCustomizationModuleChanged();

	auto& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	auto Action = MakeShared<FDMXConfigAssetAction>();
	AssetTools.RegisterAssetTypeActions(Action);

	GenerateIcons();



	CommandList = MakeShareable(new FUICommandList);

	// Create the tool widgets here without showing them.
	// When they are called to show, they create their own SDockTabs and manage their lifetimes
	VirtualLightControl = SNew(SLightControlTool, FCradleLightControlModule::Get().GetVirtualLightToolData());
	DMXControl = SNew(SDMXControlTool, FCradleLightControlModule::Get().GetDMXLightToolData());

	// Create an extension to the level editor toolbar 
	TSharedRef<FExtender> ToolbarExtender(new FExtender());
	ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, CommandList, FToolBarExtensionDelegate::CreateRaw(this, &FCradleLightControlEditorModule::AddToolBarButton));

	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	auto ex = LevelEditorModule.GetToolBarExtensibilityManager()->GetAllExtenders();
	FCoreDelegates::OnEnginePreExit.AddRaw(this, &FCradleLightControlEditorModule::OnEngineExit);

	// Ensure that slate throttling is disabled
	// If it is enabled, interacting with any slate widgets will freeze the main viewport
	// until the interaction is finished. This makes editing light properties more cumbersome if not disabled.
	//IConsoleManager::Get().FindConsoleVariable(TEXT("Slate.bAllowThrottling"))->Set(false);

	EditorNetworkInterface = MakeUnique<FDirectLightEditorNetwork>(VirtualLightControl->GetEditorData(), DMXControl->GetEditorData());
	FCradleLightControlModule::Get().EditorNetworkInterface = EditorNetworkInterface.Get();
}

void FCradleLightControlEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (VirtualLightControl)
		VirtualLightControl->PreDestroy();

	if (DMXControl)
		DMXControl->PreDestroy();

}

void FCradleLightControlEditorModule::AddToolBarButton(FToolBarBuilder& ToolbarBuilder)
{
	FUIAction Action;
	Action.ExecuteAction = FExecuteAction::CreateRaw(this, &FCradleLightControlEditorModule::OnToolBarButtonClicked);
	ToolbarBuilder.AddToolBarButton(Action, NAME_None, FText::FromString("Cradle Light Control"));
}

void FCradleLightControlEditorModule::OnToolBarButtonClicked()
{
	// I could not find a guaranteed, engine provided way to ensure that the button can spawn the tab multiple times
	// while also not allowing for the tab to be spawned multiple times simultaneously
	// So we only try to spawn the tab if one doesn't already exist, otherwise we just draw the user's attention to the existing one
	if (!LightTab)
	{
		RegisterTabSpawner();
		FGlobalTabmanager::Get()->TryInvokeTab(FTabId("VirtualLightControl"));
	}
	else
		LightTab->DrawAttention();

	if (!DMXTab)
	{
		RegisterDMXTabSpawner();
		FGlobalTabmanager::Get()->TryInvokeTab(FTabId("DMXControl"));
	}
}

void FCradleLightControlEditorModule::OnEngineExit()
{
	if (VirtualLightControl)
		VirtualLightControl->PreDestroy();
	if (DMXControl)
		DMXControl->PreDestroy();

	VirtualLightControl.Reset();
	DMXControl.Reset();
}


bool FCradleLightControlEditorModule::OpenFileDialog(FString Title, void* NativeWindowHandle, FString DefaultPath, uint32 Flags,
                                                     FString FileTypeList, TArray<FString>& OutFilenames)
{
	IDesktopPlatform* Platform = FDesktopPlatformModule::Get();
	return Platform->OpenFileDialog(NativeWindowHandle, Title, DefaultPath, "", FileTypeList, Flags, OutFilenames);
}

bool FCradleLightControlEditorModule::SaveFileDialog(FString Title, void* NativeWindowHandle, FString DefaultPath, uint32 Flags,
	FString FileTypeList, TArray<FString>& OutFilenames)
{
	IDesktopPlatform* Platform = FDesktopPlatformModule::Get();
	return Platform->SaveFileDialog(NativeWindowHandle, Title, DefaultPath, "", FileTypeList, Flags, OutFilenames);
}

FCradleLightControlEditorModule& FCradleLightControlEditorModule::Get()
{
	auto& Module = FModuleManager::GetModuleChecked<FCradleLightControlEditorModule>("CradleLightControlEditor");
	return Module;
}

TArray<UBaseLight*> FCradleLightControlEditorModule::GetLightsFromHandles(TArray<UItemHandle*> Handles)
{
	TArray<UBaseLight*> Lights;
	Lights.Reserve(Handles.Num());

	for (auto& Handle : Handles)
	{
		Lights.Add(Handle->Item);
	}

	return Lights;
}

void FCradleLightControlEditorModule::OpenGelPalette(FGelPaletteSelectionCallback SelectionCallback)
{
	if (!GelPalette)
	{
		GelPalette = SNew(SGelPaletteWidget);
	}

	if (!GelPaletteWindow)
	{

		GelPaletteWindow = SNew(SWindow)
			.ClientSize(FVector2D(640.0f, 480.0f))
			.Title(FText::FromString("Light Gel Palette"))
			.CreateTitleBar(true)
			[
				GelPalette->AsShared()
			];
		GelPaletteWindow = FSlateApplication::Get().AddWindow(GelPaletteWindow.ToSharedRef());
		//.IsPopupWindow(true)

		GelPalette->Window = GelPaletteWindow;

		//GelPaletteWindow->ShowWindow();
	}

	if (!GelPaletteWindow->IsVisible())
	{
		GelPalette->SelectionCallback = SelectionCallback;
		GelPaletteWindow->ShowWindow();
	}
	else
	{
		GelPalette->SelectionCallback = SelectionCallback;
		GelPaletteWindow->FlashWindow();
		//GelPaletteWindow->DrawAttention()
	}


}

void FCradleLightControlEditorModule::CloseGelPalette()
{
	GelPaletteWindow->HideWindow();
}

void FCradleLightControlEditorModule::RegisterTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner("VirtualLightControl", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args)
		{			
			return VirtualLightControl->Show();
		}));
}

void FCradleLightControlEditorModule::RegisterDMXTabSpawner()
{

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner("DMXControl", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args)
		{
			return DMXControl->Show();
		}));

}



void FCradleLightControlEditorModule::GenerateIcons()
{
	FLinearColor OffTint(0.2f, 0.2f, 0.2f, 0.5f);
	FLinearColor UndeterminedTint(0.8f, 0.8f, 0.0f, 0.5f);
	Icons.Emplace(SkyLightOn, *FClassIconFinder::FindThumbnailForClass(ASkyLight::StaticClass()));
	Icons.Emplace(SkyLightOff, Icons[SkyLightOn]);
	Icons[SkyLightOff].TintColor = OffTint;
	Icons.Emplace(SkyLightUndetermined, Icons[SkyLightOn]);
	Icons[SkyLightUndetermined].TintColor = UndeterminedTint;

	Icons.Emplace(DirectionalLightOn, *FClassIconFinder::FindThumbnailForClass(ADirectionalLight::StaticClass()));
	Icons.Emplace(DirectionalLightOff, Icons[DirectionalLightOn]);
	Icons[DirectionalLightOff].TintColor = OffTint;
	Icons.Emplace(DirectionalLightUndetermined, Icons[DirectionalLightOn]);
	Icons[DirectionalLightUndetermined].TintColor = UndeterminedTint;

	Icons.Emplace(SpotLightOn, *FClassIconFinder::FindThumbnailForClass(ASpotLight::StaticClass()));
	Icons.Emplace(SpotLightOff, Icons[SpotLightOn]);
	Icons[SpotLightOff].TintColor = OffTint;
	Icons.Emplace(SpotLightUndetermined, Icons[SpotLightOn]);
	Icons[SpotLightUndetermined].TintColor = UndeterminedTint;

	Icons.Emplace(PointLightOn, *FClassIconFinder::FindThumbnailForClass(APointLight::StaticClass()));
	Icons.Emplace(PointLightOff, Icons[PointLightOn]);
	Icons[PointLightOff].TintColor = OffTint;
	Icons.Emplace(PointLightUndetermined, Icons[PointLightOn]);
	Icons[PointLightUndetermined].TintColor = UndeterminedTint;

	Icons.Emplace(GeneralLightOn, Icons[PointLightOn]);
	Icons.Emplace(GeneralLightOff, Icons[PointLightOff]);
	Icons.Emplace(GeneralLightUndetermined, Icons[PointLightUndetermined]);

	Icons.Emplace(FolderClosed, *FEditorStyle::GetBrush("ContentBrowser.ListViewFolderIcon.Mask"));
	Icons.Emplace(FolderOpened, *FEditorStyle::GetBrush("ContentBrowser.ListViewFolderIcon.Base"));

	for (auto& Icon : Icons)
	{
		Icon.Value.SetImageSize(FVector2D(24.0f));
	}
}

FCheckBoxStyle FCradleLightControlEditorModule::MakeCheckboxStyleForType(uint8 IconType)
{
	check(IconType != ELightType::Invalid);

	// ELightType and EIconType are ordered in such a manner that
	// the light type in EIconType changes every 3 enums, with the order always being Off, On and Undetermined.
	// Because of this, IconType * 3 + 0/1/2 will give us the Off/On/Undetermined icon for the given item type.

	FCheckBoxStyle CheckBoxStyle;
	CheckBoxStyle.CheckedImage = Icons[StaticCast<EIconType>(IconType * 3 + 1)];
	CheckBoxStyle.CheckedHoveredImage = Icons[StaticCast<EIconType>(IconType * 3 + 1)];
	CheckBoxStyle.CheckedPressedImage = Icons[StaticCast<EIconType>(IconType * 3 + 1)];

	CheckBoxStyle.UncheckedImage = Icons[StaticCast<EIconType>(IconType * 3 + 0)];
	CheckBoxStyle.UncheckedHoveredImage = Icons[StaticCast<EIconType>(IconType * 3 + 0)];
	CheckBoxStyle.UncheckedPressedImage = Icons[StaticCast<EIconType>(IconType * 3 + 0)];

	CheckBoxStyle.UndeterminedImage = Icons[StaticCast<EIconType>(IconType * 3 + 2)];
	CheckBoxStyle.UndeterminedHoveredImage = Icons[StaticCast<EIconType>(IconType * 3 + 2)];
	CheckBoxStyle.UndeterminedPressedImage = Icons[StaticCast<EIconType>(IconType * 3 + 2)];

	return CheckBoxStyle;
}

FSlateBrush& FCradleLightControlEditorModule::GetIcon(EIconType Icon)
{
	return Icons[Icon];
}



#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCradleLightControlEditorModule, CradleLightControl)