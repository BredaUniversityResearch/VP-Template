#pragma once

#include "BMCCCommandPayload.h"
#include "BMCCCommandIdentifier.h"
#include "BMCCFixed16.h"

#include "BMCCVendorSpecific.generated.h"

USTRUCT(BlueprintType)
struct FBMCCVendorSpecific_CanonLens: public FBMCCCommandPayloadBase
{
	GENERATED_BODY()
	
	static constexpr FBMCCCommandIdentifier Identifier = FBMCCCommandIdentifier(12, 12);

	FBMCCVendorSpecific_CanonLens() = default;
	FBMCCVendorSpecific_CanonLens(const TArrayView<uint8>& a_Data)
		: InfoString(FUTF8ToTCHAR(reinterpret_cast<ANSICHAR*>(a_Data.GetData()), a_Data.Num()).Get())
	{
	}

	FString InfoString;
};
