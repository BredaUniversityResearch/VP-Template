#include "BaseLightPropertyChangeSpeaker.h"
#include "BaseLightPropertyChangeListener.h"

#include "CradleLightControl.h"



void FBaseLightPropertyChangeSpeaker::SendLightPropertyChangeEvent(TArray<UItemHandle*>& AffectedLights, FBaseLightPropertyChangeListener::EProperty Property, float Value)
{
	FCradleLightControlModule::GetLightPropertyChangeListener().OnPropertiesChanged(AffectedLights, Property, Value);
}
