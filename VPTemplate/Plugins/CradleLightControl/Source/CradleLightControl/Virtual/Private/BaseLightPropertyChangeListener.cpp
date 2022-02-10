#include "BaseLightPropertyChangeListener.h"

#include "BaseLight.h"

void FBaseLightPropertyChangeListener::OnPropertiesChanged(TArray<UBaseLight*>& AffectedLights, FBaseLightPropertyChangeListener::EProperty Property, float Value)
{
	
	for (auto& LightHandle : AffectedLights)
	{
		LightHandle->BeginTransaction();
		switch (Property)
		{
			case State:
				LightHandle->SetEnabled(Value > 0.9f);
				break;
			case Intensity:
				LightHandle->SetLightIntensity(Value);
				break;
			case Hue:
				LightHandle->SetHue(Value);
				break;
			case Saturation:
				LightHandle->SetSaturation(Value);
				break;
			case UseTemperature:
				LightHandle->SetUseTemperature(Value > 0.9f);
				break;
			case Temperature:
				LightHandle->SetTemperature(Value);
				break;
			case Horizontal:
				LightHandle->AddHorizontal(Value);
				break;
			case Vertical:
				LightHandle->AddVertical(Value);
				break;
			case OuterConeAngle:
				LightHandle->SetOuterConeAngle(Value);
				break;
			case InnerConeAngle:
				LightHandle->SetInnerConeAngle(Value);
				break;
		}
	}
}
