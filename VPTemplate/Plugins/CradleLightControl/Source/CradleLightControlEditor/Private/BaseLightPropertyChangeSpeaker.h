#pragma once
#include "BaseLightPropertyChangeListener.h"

class UItemHandle;

class FBaseLightPropertyChangeSpeaker
{
public:
	

	virtual void SendLightPropertyChangeEvent(TArray<UItemHandle*>& AffectedLights, FBaseLightPropertyChangeListener::EProperty Property, float Value);
private:

};