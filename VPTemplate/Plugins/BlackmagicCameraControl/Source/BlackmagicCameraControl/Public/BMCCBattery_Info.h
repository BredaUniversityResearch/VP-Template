#pragma once
#include "BMCCCommandPayload.h"
#include "BMCCCommandIdentifier.h"

#include "BMCCBattery_Info.generated.h"

USTRUCT(BlueprintType)
struct FBMCCBattery_Info: public FBMCCCommandPayloadBase
{
	GENERATED_BODY()

	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(9, 0);

	int16 BatteryVoltage_mV;
	int16 BatteryPercentage;
	int16 Unknown;
};
static_assert(sizeof(FBMCCBattery_Info) == 6, "FBMCCBattery_Info is expected to contain 3 int16s");
