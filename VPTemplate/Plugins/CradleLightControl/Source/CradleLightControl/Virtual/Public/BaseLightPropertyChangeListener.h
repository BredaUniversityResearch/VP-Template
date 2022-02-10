#pragma once
#include "BaseLight.h"
#include "Chaos/AABB.h"


class UItemHandle;


class CRADLELIGHTCONTROL_API FBaseLightPropertyChangeListener
{
public:
	enum EProperty
	{
		State,
		Intensity,
		Hue,
		Saturation,
		UseTemperature,
		Temperature,
		Horizontal,
		Vertical,
		OuterConeAngle,
		InnerConeAngle,

	};
	
	void OnPropertiesChanged(TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value);

private:
	

};