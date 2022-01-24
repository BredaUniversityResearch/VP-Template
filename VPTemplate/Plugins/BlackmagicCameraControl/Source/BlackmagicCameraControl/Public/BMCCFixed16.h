#pragma once

#include "BMCCFixed16.generated.h"

USTRUCT(BlueprintType)
struct FBMCCFixed16
{
	GENERATED_BODY()

	static constexpr float Exponent = (1 << 11);
public:
	FBMCCFixed16() = default;
	explicit FBMCCFixed16(float Value);

	float AsFloat() const;

private:
	UPROPERTY(VisibleAnywhere)
	int16 Data{ 0 };
};

static_assert(sizeof(FBMCCFixed16) == sizeof(int16));