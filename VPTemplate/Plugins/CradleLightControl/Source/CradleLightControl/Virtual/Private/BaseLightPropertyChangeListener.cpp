#include "BaseLightPropertyChangeListener.h"

#include "BaseLight.h"
#include "ItemHandle.h"

void FBaseLightPropertyChangeListener::OnPropertiesChanged(TArray<UItemHandle*>& AffectedLights, FBaseLightPropertyChangeListener::EProperty Property, float Value)
{
	if (GEditor)
	{
		//GEditor->BeginTransaction(FText::FromString("Light control light property changed"));
	}
	for (auto& LightHandle : AffectedLights)
	{
		LightHandle->BeginTransaction();
		switch (Property)
		{
			case State:
				LightHandle->Item->SetEnabled(Value > 0.9f);
				break;
			case Intensity:
				LightHandle->Item->SetLightIntensity(Value);
				break;
			case Hue:
				LightHandle->Item->SetHue(Value);
				break;
			case Saturation:
				LightHandle->Item->SetSaturation(Value);
				break;
			case UseTemperature:
				LightHandle->Item->SetUseTemperature(Value > 0.9f);
				break;
			case Temperature:
				LightHandle->Item->SetTemperature(Value);
				break;
			case Horizontal:
				LightHandle->Item->AddHorizontal(Value);
				break;
			case Vertical:
				LightHandle->Item->AddVertical(Value);
				break;
			case OuterConeAngle:
				LightHandle->Item->SetOuterConeAngle(Value);
				break;
			case InnerConeAngle:
				LightHandle->Item->SetInnerConeAngle(Value);
				break;
		}
	}
	if (GEditor)
	{
		//GEditor->EndTransaction();
	}
}
