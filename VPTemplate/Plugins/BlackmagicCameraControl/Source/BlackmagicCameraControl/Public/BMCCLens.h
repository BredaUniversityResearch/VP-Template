#pragma once

#include "BMCCCommandPayload.h"
#include "BMCCCommandIdentifier.h"
#include "BMCCFixed16.h"

#include "BMCCLens.generated.h"

USTRUCT(BlueprintType)
struct FBMCCLens_Focus : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 0);

	UPROPERTY(BlueprintReadWrite)
		FBMCCFixed16 Focus; // 0.0 = near, 1.0 = far
};
static_assert(sizeof(FBMCCLens_Focus) == 2);

USTRUCT(BlueprintType)
struct FBMCCLens_TriggerAutoFocus : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 1);
};

USTRUCT(BlueprintType)
struct FBMCCLens_ApertureFStop : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 2);

	UPROPERTY(BlueprintReadWrite)
	FBMCCFixed16 Aperture; //F-Number = sqrt(2^Aperture), -1..16
};

USTRUCT(BlueprintType)
struct FBMCCLens_ApertureNormalized : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 3);

	UPROPERTY(BlueprintReadWrite)
		FBMCCFixed16 Aperture; //0..1
};

USTRUCT(BlueprintType)
struct FBMCCLens_ApertureOrdinal : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 4);

	UPROPERTY(BlueprintReadWrite)
		FBMCCFixed16 Aperture; //0..N where N is maximum ordinal aperture value
};

USTRUCT(BlueprintType)
struct FBMCCLens_TriggerInstantAutoAperture : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 5);
};

USTRUCT(BlueprintType)
struct FBMCCLens_OpticalImageStabilization : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 6);

	UPROPERTY(BlueprintReadWrite)
		bool ImageStabilizationEnabled;
};
static_assert(sizeof(FBMCCLens_OpticalImageStabilization) == 1);

USTRUCT(BlueprintType)
struct FBMCCLens_SetAbsoluteZoomMm : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 7);

	UPROPERTY(VisibleAnywhere)
	int16 ZoomInMm;
};

USTRUCT(BlueprintType)
struct FBMCCLens_SetAbsoluteZoomNormalized : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 8);

	UPROPERTY(BlueprintReadWrite)
		FBMCCFixed16 ZoomNormalized; // 0..1
};

USTRUCT(BlueprintType)
struct FBMCCLens_SetContinuousZoom : public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

		static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(0, 9);

	UPROPERTY(BlueprintReadWrite)
		FBMCCFixed16 ZoomSpeed; // -1..1. -1 = zoom wider fast, 0.0 = stop, +1 zoom tele fast
};
