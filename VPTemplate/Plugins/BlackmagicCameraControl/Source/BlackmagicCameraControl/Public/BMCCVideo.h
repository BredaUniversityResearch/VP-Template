#pragma once

#include "BMCCCommandPayload.h"
#include "BMCCCommandIdentifier.h"
#include "BMCCFixed16.h"

#include "BMCCVideo.generated.h"

USTRUCT(BlueprintType)
struct FBMCCVideo_VideoMode : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()
	
	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(1, 0);

	enum class EDimensions : uint8_t
	{
		D_NTSC = 0,
		D_PAL = 1,
		D_720 = 2,
		D_1080 = 3,
		D_2k = 4,
		D_2k_DCI = 5,
		D_UHD = 6
	};

	enum class EColorSpace : uint8_t
	{
		YUV = 0,
	};

	int8 Framerate;
	bool MRate {false};
	EDimensions Dimensions;
	bool Interlaced{ false };
	EColorSpace ColorSpace {EColorSpace::YUV };

};
static_assert(sizeof(FBMCCVideo_VideoMode) == 5);

USTRUCT(BlueprintType)
struct FBMCCVideo_Gain : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(1, 1);

	int8 ISO; //Iso value divided by 100. 1 = 100, 2 = 200, 4 = 400
};

//Missing 1.2 to 1.8

USTRUCT(BlueprintType)
struct	FBMCCVideo_RecordingFormat : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(1, 9);

	enum class EFlags: int16
	{
		FileMRate = (1 << 0),
		SensorMRate = (1 << 1),
		SensorOffSpeed = (1 << 2),
		Interlaced = (1 << 3),
		WindowedMode = (1 << 4)
	};

	int16 FileFrameRate{ 0 };
	int16 SensorFrameRate{ 0 };
	int16 FrameWidthPixels{ 0 };
	int16 FrameHeightPixels{ 0 };
	EFlags Flags{};
};
