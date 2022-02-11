#pragma once
#include "LightRuntimeNetwork.h"

class UItemHandle;

class CRADLELIGHTCONTROL_API ILightEditorNetwork
{
public:
	ILightEditorNetwork(class UEditorData* InVirtualLightEditorData, UEditorData* InDMXLightEditorData);

	virtual ~ILightEditorNetwork() {}

	virtual void SendLightPropertyChangeEvent(EDataSet TargetDataSet, TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value) = 0;

	virtual void OnNewVirtualLightSpawned(UBaseLight* NewLight) = 0;

	virtual void RemoveInvalidVirtualLightsFromEditor(TArray<UBaseLight*> LightsToRemove) = 0;

protected:

	UEditorData* VirtualLightEditorData;
	UEditorData* DMXLightEditorData;


};