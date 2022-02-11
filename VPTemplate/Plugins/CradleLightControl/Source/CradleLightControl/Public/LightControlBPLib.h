// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LightControlBPLib.generated.h"

/**
 * Blueprint function library to allow for the plugin's data to be accessed via blueprints
 */
UCLASS(meta = (ScriptName="CradleLightControl"))
class CRADLELIGHTCONTROL_API ULightControlBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CradleLightControl")
		static class UToolData* GetDMXLightToolData();

	UFUNCTION(BlueprintPure, Category = "CraldeLightControl")
		static class UToolData* GetVirtualLightToolData();

};
