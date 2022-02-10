#include "BaseLightPropertyChangeSpeaker.h"
#include "BaseLightPropertyChangeListener.h"

#include "CradleLightControl.h"
#include "ItemHandle.h"


void FBaseLightPropertyChangeSpeaker::SendLightPropertyChangeEvent(TArray<UItemHandle*>& AffectedLights, FBaseLightPropertyChangeListener::EProperty Property, float Value)
{
	TArray<UBaseLight*> Lights;
	for (auto& AffectedLight : AffectedLights)
	{
		Lights.Add(AffectedLight->Item);
	}
	FCradleLightControlModule::GetLightPropertyChangeListener().OnPropertiesChanged(Lights, Property, Value);
}
