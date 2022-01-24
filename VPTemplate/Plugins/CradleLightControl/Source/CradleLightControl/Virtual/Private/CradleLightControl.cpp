// Copyright Epic Games, Inc. All Rights Reserved.

#include "CradleLightControl.h"

#include "AssetToolsModule.h"
#include "LevelEditor.h"
#include "DMXConfigAsset.h"

#include "DesktopPlatformModule.h"
#include "DMXLight.h"
#include "IDesktopPlatform.h"
#include "ItemHandle.h"

#include "ToolData.h"
#include "VirtualLight.h"
#include "Engine/Light.h"

#include "Kismet/GameplayStatics.h"

// Core module for the plugin
// Main purpose of the module is to create and manage the data that the plugin uses it,
// while the UI comes from the Editor module (CradleLightControlEditor)
// This data can still be accessed via blueprints at runtime

#define LOCTEXT_NAMESPACE "FCradleLightControlModule"

DEFINE_LOG_CATEGORY(LogCradleLightControl)

void FCradleLightControlModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Initialization of the UToolData objects for both virtual and DMX fixture lights

	VirtualLightToolData = NewObject<UToolData>();
	VirtualLightToolData->ItemClass = UVirtualLight::StaticClass();

	DMXLightToolData = NewObject<UToolData>();
	DMXLightToolData->ItemClass = UDMXLight::StaticClass();

	// Since these UObjects are being created and managed by a non-UObject, we need to manually register them
	// in the garbage collector's hierarchy. Otherwise they will get garbage collected at some point while still in use.
	VirtualLightToolData->AddToRoot();
	DMXLightToolData->AddToRoot();


	if (GEngine && GEngine->IsEditor())
	{
		FModuleManager::Get().LoadModule("CradleLightControlEditor");

		FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FCradleLightControlModule::OnWorldInitialized);

		FWorldDelegates::OnWorldCleanup.AddRaw(this, &FCradleLightControlModule::OnWorldCleanup);

		
	}
	
}

void FCradleLightControlModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// We remove the objects from the garbage collector's registry so that they can be garbage collected

	VirtualLightToolData->RemoveFromRoot();
	DMXLightToolData->RemoveFromRoot();

}

FCradleLightControlModule& FCradleLightControlModule::Get()
{
	auto& Module = FModuleManager::GetModuleChecked<FCradleLightControlModule>("CradleLightControl");

	return Module;
}

UToolData* FCradleLightControlModule::GetVirtualLightToolData()
{
	return VirtualLightToolData;
}

UToolData* FCradleLightControlModule::GetDMXLightToolData()
{
	return DMXLightToolData;
}

void FCradleLightControlModule::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, "FWorldDelegates::OnPostDuplicate called");

	TArray<AActor*> Lights;
	UGameplayStatics::GetAllActorsOfClass(World, ALight::StaticClass(), Lights);


	for (auto& RootItem : VirtualLightToolData->RootItems)
	{
		RootItem->UpdateVirtualLights(Lights);
	}
}

void FCradleLightControlModule::OnWorldCleanup(UWorld*, bool, bool)
{
	for (auto& RootItem : VirtualLightToolData->RootItems)
	{
		RootItem->RestoreVirtualLightReferences();
	}
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCradleLightControlModule, CradleLightControl)