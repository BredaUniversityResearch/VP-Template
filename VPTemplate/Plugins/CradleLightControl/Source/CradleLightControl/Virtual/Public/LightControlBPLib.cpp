// Fill out your copyright notice in the Description page of Project Settings.


#include "LightControlBPLib.h"

#include "CradleLightControl.h"


UToolData* ULightControlBPLib::GetDMXLightToolData()
{
	auto& Module = FModuleManager::GetModuleChecked<FCradleLightControlModule>("CradleLightControl");

	return Module.GetDMXLightToolData();
}

UToolData* ULightControlBPLib::GetVirtualLightToolData()
{
	auto& Module = FModuleManager::GetModuleChecked<FCradleLightControlModule>("CradleLightControl");

	return Module.GetVirtualLightToolData();
}
