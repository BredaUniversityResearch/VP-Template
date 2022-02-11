#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EProperty : uint8
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

UENUM()
enum class EDataSet : uint8
{
	VirtualLights,
	DMXLights
};