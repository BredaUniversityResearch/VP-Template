#include "RemoteControlAutoExposeModule.h"

#include "ContentBrowserModule.h"
#include "RemoteControlAssetActionExtension.h"

#define LOCTEXT_NAMESPACE "FRemoteControlAutoExposeModule"

void FRemoteControlAutoExposeModule::StartupModule()
{
	m_Extender = MakeUnique<FRemoteControlAssetActionExtension>();

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(m_Extender.Get(), &FRemoteControlAssetActionExtension::CreateMenuExtender));
}

void FRemoteControlAutoExposeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRemoteControlAutoExposeModule, RemoteControlAutoExposeModule)