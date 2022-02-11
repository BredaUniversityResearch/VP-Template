#pragma once
#include "LightEditorNetwork.h"
#include "LightRuntimeNetwork.h"


class UItemHandle;


class FDirectLightEditorNetwork : public ILightEditorNetwork
{
public:
	FDirectLightEditorNetwork(class UEditorData* InVirtualLightEditorData, UEditorData* InDMXLightEditorData);

	~FDirectLightEditorNetwork() {}

	virtual void SendLightPropertyChangeEvent(EDataSet TargetDataSet, TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value) override;

	virtual void OnNewVirtualLightSpawned(UBaseLight* NewLight) override;

	virtual void RemoveInvalidVirtualLightsFromEditor(TArray<UBaseLight*> LightsToRemove) override;

protected:
	


};