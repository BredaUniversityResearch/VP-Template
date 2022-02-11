#include "DirectLightRuntimeNetwork.h"

#include "BaseLight.h"

#include "CradleLightControl.h"
#include "LightEditorNetwork.h"

void FDirectLightRuntimeNetwork::OnPropertiesChanged(EDataSet, TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value)
{

	for (auto& LightHandle : AffectedLights)
	{
		LightHandle->BeginTransaction();
		switch (Property)
		{
		case EProperty::State:
			LightHandle->SetEnabled(Value > 0.9f);
			break;
		case EProperty::Intensity:
			LightHandle->SetLightIntensity(Value);
			break;
		case EProperty::Hue:
			LightHandle->SetHue(Value);
			break;
		case EProperty::Saturation:
			LightHandle->SetSaturation(Value);
			break;
		case EProperty::UseTemperature:
			LightHandle->SetUseTemperature(Value > 0.9f);
			break;
		case EProperty::Temperature:
			LightHandle->SetTemperature(Value);
			break;
		case EProperty::Horizontal:
			LightHandle->AddHorizontal(Value);
			break;
		case EProperty::Vertical:
			LightHandle->AddVertical(Value);
			break;
		case EProperty::OuterConeAngle:
			LightHandle->SetOuterConeAngle(Value);
			break;
		case EProperty::InnerConeAngle:
			LightHandle->SetInnerConeAngle(Value);
			break;
		}
	}
}

void FDirectLightRuntimeNetwork::OnDMXLightAdded()
{
}

void FDirectLightRuntimeNetwork::OnNewLightStateLoaded(EDataSet TargetDataSet, FString JsonString)
{
}

FString FDirectLightRuntimeNetwork::RequestDataForLight(EDataSet TargetDataSet, FString LightName)
{
	return "";
}

void FDirectLightRuntimeNetwork::OnLightActorSpawned(UBaseLight* NewLight)
{
	FCradleLightControlModule::GetEditorNetworkInterface().OnNewVirtualLightSpawned(NewLight);
}

void FDirectLightRuntimeNetwork::RemoveInvalidVirtualLightsFromEditor(TArray<UBaseLight*> LightsToRemove)
{
	FCradleLightControlModule::GetEditorNetworkInterface().RemoveInvalidVirtualLightsFromEditor(LightsToRemove);
}
