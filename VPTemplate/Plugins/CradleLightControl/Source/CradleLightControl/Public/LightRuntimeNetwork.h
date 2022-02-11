#pragma once
#include "BaseLight.h"
#include "LightControlNetworkEnums.h"


class UItemHandle;


class CRADLELIGHTCONTROL_API ILightRuntimeNetwork
{
public:

	virtual ~ILightRuntimeNetwork() {};
	
	virtual void OnPropertiesChanged(EDataSet TargetDataSet, TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value) = 0;

	virtual void OnDMXLightAdded() = 0;

	virtual void OnNewLightStateLoaded(EDataSet TargetDataSet, FString JsonString) = 0;

	virtual FString RequestDataForLight(EDataSet TargetDataSet, FString LightName) = 0;

	virtual void OnLightActorSpawned(UBaseLight* NewLight) = 0;

	virtual void RemoveInvalidVirtualLightsFromEditor(TArray<UBaseLight*> LightsToRemove) = 0;


private:
	

};