#include "BMCCFixed16.h"

#include "BlackmagicCameraControl.h"

FBMCCFixed16::FBMCCFixed16(float Value)
	: Data(static_cast<int16>(Value * Exponent))
{
	if (Value > 16.0f || Value <= -16.0f)
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Value out of range for Fixed16 value. Value was %f. Allowed range is (-16..16]"), Value);
	}
}

float FBMCCFixed16::AsFloat() const
{
	return static_cast<float>(Data) / Exponent;
}
