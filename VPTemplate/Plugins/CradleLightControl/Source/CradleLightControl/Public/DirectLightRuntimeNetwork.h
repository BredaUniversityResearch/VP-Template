#pragma once
#include "BaseLight.h"
#include "LightControlNetworkEnums.h"
#include "LightRuntimeNetwork.h"

class UItemHandle;

class FDirectLightRuntimeNetwork : public ILightRuntimeNetwork
{
public:


	virtual void OnPropertiesChanged(EDataSet TargetDataSet, TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value) override;

	virtual void OnDMXLightAdded() override;

	virtual void OnNewLightStateLoaded(EDataSet TargetDataSet, FString JsonString) override;

	virtual FString RequestDataForLight(EDataSet TargetDataSet, FString LightName) override;

	virtual void OnLightActorSpawned(UBaseLight* NewLight) override;

	virtual void RemoveInvalidVirtualLightsFromEditor(TArray<UBaseLight*> LightsToRemove) override;


private:


};