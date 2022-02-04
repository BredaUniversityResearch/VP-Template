#pragma once


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
	
	void OnPropertiesChanged(TArray<UItemHandle*>& AffectedLights, EProperty Property, float Value);

private:
	

};