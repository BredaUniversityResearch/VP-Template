// Copyright Epic Games, Inc. All Rights Reserved.

#include "CradleLightControl.h"

#include "DirectLightRuntimeNetwork.h"
#include "LightEditorNetwork.h"
#include "DMXConfigAsset.h"

#include "DMXLight.h"

#include "ToolData.h"
#include "VirtualLight.h"
#include "Engine/Light.h"

#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"

#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"

#include "Kismet/GameplayStatics.h"

// Core module for the plugin
// Main purpose of the module is to create and manage the data that the plugin uses it,
// while the UI comes from the Editor module (CradleLightControlEditor)
// This data can still be accessed via blueprints at runtime

#define LOCTEXT_NAMESPACE "FCradleLightControlModule"

DEFINE_LOG_CATEGORY(LogCradleLightControl)

FCradleLightControlModule::~FCradleLightControlModule()
{
	RuntimeNetworkInterface.Reset();
}

void FCradleLightControlModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Initialization of the UToolData objects for both virtual and DMX fixture lights

	VirtualLightToolData = NewObject<UToolData>();
	VirtualLightToolData->DataName = "VirtualLight";
	VirtualLightToolData->ItemClass = UVirtualLight::StaticClass();

	DMXLightToolData = NewObject<UToolData>();
	DMXLightToolData->DataName = "DMXLights";
	DMXLightToolData->ItemClass = UDMXLight::StaticClass();

	RuntimeNetworkInterface = MakeUnique<FDirectLightRuntimeNetwork>();

	// Since these UObjects are being created and managed by a non-UObject, we need to manually register them
	// in the garbage collector's hierarchy. Otherwise they will get garbage collected at some point while still in use.
	VirtualLightToolData->AddToRoot();
	DMXLightToolData->AddToRoot();


	FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FCradleLightControlModule::OnWorldInitialized);

	FWorldDelegates::OnWorldCleanup.AddRaw(this, &FCradleLightControlModule::OnWorldCleanup);
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

void FCradleLightControlModule::UpdateDataForLight(UBaseLight* BaseLight)
{
    check(BaseLight);
    auto VirtualLight = Cast<UVirtualLight>(BaseLight);
    FLinearColor RGB;

    BaseLight->Intensity = 0.0f;
    BaseLight->Saturation = 0.0f;
    BaseLight->Temperature = 0.0f;

    if (BaseLight->Type == ELightType::SkyLight)
    {
        RGB = VirtualLight->SkyLight->GetLightComponent()->GetLightColor();
        BaseLight->bIsEnabled = VirtualLight->SkyLight->GetLightComponent()->IsVisible();
    }
    else
    {
        ALight* LightPtr = Cast<ALight>(VirtualLight->ActorPtr);
        RGB = LightPtr->GetLightColor();
        BaseLight->bIsEnabled = LightPtr->GetLightComponent()->IsVisible();
    }
    auto HSV = RGB.LinearRGBToHSV();
    BaseLight->Saturation = HSV.G;

    // If Saturation is 0, the color is white. The RGB => HSV conversion calculates the Hue to be 0 in that case, even if it's not supposed to be.
    // Do this to preserve the Hue previously used rather than it getting reset to 0.
    if (BaseLight->Saturation != 0.0f)
        BaseLight->Hue = HSV.R;

    if (BaseLight->Type == ELightType::PointLight)
    {
        auto Comp = VirtualLight->PointLight->PointLightComponent;
        BaseLight->Intensity = Comp->Intensity;
    }
    else if (BaseLight->Type == ELightType::SpotLight)
    {
        auto Comp = VirtualLight->SpotLight->SpotLightComponent;
        BaseLight->Intensity = Comp->Intensity;
    }

    if (BaseLight->Type != ELightType::SkyLight)
    {
        auto LightPtr = Cast<ALight>(VirtualLight->ActorPtr);
        auto LightComp = LightPtr->GetLightComponent();
        BaseLight->bUseTemperature = LightComp->bUseTemperature;
        BaseLight->Temperature = LightComp->Temperature;

        VirtualLight->bCastShadows = LightComp->CastShadows;
    }
    else
    {
        VirtualLight->bCastShadows = VirtualLight->SkyLight->GetLightComponent()->CastShadows;
    }

    auto CurrentFwd = FQuat::MakeFromEuler(FVector(0.0f, BaseLight->Vertical, BaseLight->Horizontal)).GetForwardVector();
    auto ActorQuat = VirtualLight->ActorPtr->GetTransform().GetRotation().GetNormalized();
    auto ActorFwd = ActorQuat.GetForwardVector();

    if (CurrentFwd.Equals(ActorFwd))
    {
        auto Euler = ActorQuat.Euler();
        BaseLight->Horizontal = Euler.Z;
        BaseLight->Vertical = Euler.Y;
    }


    if (BaseLight->Type == ELightType::SpotLight)
    {
        BaseLight->InnerAngle = VirtualLight->SpotLight->SpotLightComponent->InnerConeAngle;
        BaseLight->OuterAngle = VirtualLight->SpotLight->SpotLightComponent->OuterConeAngle;
    }

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
	TArray<AActor*> Lights;
	UGameplayStatics::GetAllActorsOfClass(World, ALight::StaticClass(), Lights);

	VirtualLightToolData->LoadMetaData();
	DMXLightToolData->LoadMetaData();

	for (auto& RootItem : VirtualLightToolData->Lights)
	{
		Cast<UVirtualLight>(RootItem)->UpdateVirtualLights(Lights);
	}

    ActorSpawnedDelegate = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &FCradleLightControlModule::OnActorSpawned));

    World->GetTimerManager().SetTimer(VirtualLightVerificationTimerHandle, FTimerDelegate::CreateRaw(this, &FCradleLightControlModule::VerifyVirtualLightsData), 0.2f, true, 2.0f);
}

void FCradleLightControlModule::OnWorldCleanup(UWorld* World, bool, bool)
{
	for (auto& RootItem : VirtualLightToolData->Lights)
	{
		Cast<UVirtualLight>(RootItem)->RestoreVirtualLightReferences();
	}
    World->RemoveOnActorSpawnedHandler(ActorSpawnedDelegate);
    ActorSpawnedDelegate.Reset();
    World->GetTimerManager().ClearTimer(VirtualLightVerificationTimerHandle);
    VirtualLightVerificationTimerHandle.Invalidate();
}

void FCradleLightControlModule::OnActorSpawned(AActor* SpawnedActor)
{
    auto Type = Invalid;

    if (Cast<ASkyLight>(SpawnedActor))
        Type = ELightType::SkyLight;
    else if (Cast<ASpotLight>(SpawnedActor))
        Type = ELightType::SpotLight;
    else if (Cast<ADirectionalLight>(SpawnedActor))
        Type = ELightType::DirectionalLight;
    else if (Cast<APointLight>(SpawnedActor))
        Type = ELightType::PointLight;

    if (Type != Invalid)
    {
        auto NewLight = VirtualLightToolData->AddItem();
        NewLight->Type = Type;
        NewLight->Name = SpawnedActor->GetName();

        auto Item = Cast<UVirtualLight>(NewLight);

        switch (Type)
        {
        case SkyLight:
            Item->SkyLight = Cast<ASkyLight>(SpawnedActor);
            break;
        case SpotLight:
            Item->SpotLight = Cast<ASpotLight>(SpawnedActor);
            break;
        case DirectionalLight:
            Item->DirectionalLight = Cast<ADirectionalLight>(SpawnedActor);
            break;
        case PointLight:
            Item->PointLight = Cast<APointLight>(SpawnedActor);
            break;
        }

        RuntimeNetworkInterface->OnLightActorSpawned(NewLight);

        
    }
}

void FCradleLightControlModule::VerifyVirtualLightsData()
{
    if (VirtualLightToolData->bCurrentlyLoading)
        return;

    GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, "Cleaning invalid lights");
    TArray<UBaseLight*> ToRemove;
    for (auto Light : VirtualLightToolData->Lights)
    {
        auto Item = Cast<UVirtualLight>(Light);
        if (!Item->ActorPtr || !IsValid(Item->SkyLight))
        {

        	ToRemove.Add(Light);
        }
        else
        {
            //UpdateItemData(ItemHandle->Item);
        }
    }

    RuntimeNetworkInterface->RemoveInvalidVirtualLightsFromEditor(ToRemove);

    for (auto Light : ToRemove)
    {
    	VirtualLightToolData->Lights.Remove(Light);	    
    }
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCradleLightControlModule, CradleLightControl)