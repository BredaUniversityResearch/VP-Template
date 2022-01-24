#pragma once

#include "BMCCCommandIdentifier.h"

enum class EDataType : uint8_t
{
	VoidOrBool = 0,
	Int8 = 1,
	Int16 = 2,
	Int32 = 3,
	Int64 = 4,
	Utf8String = 5,
	Signed5_11FixedPoint = 128 //int16 5:11 Fixed point
};

enum class EOperation : uint8_t
{
	Assign = 0,
	OffsetOrToggle = 1
};

struct BMCCCommandHeader
{
	BMCCCommandHeader() = default;
	BMCCCommandHeader(const FBMCCCommandIdentifier& Identifier)
		: Identifier(Identifier)
	{
	}

	FBMCCCommandIdentifier Identifier;
	EDataType DataType{ EDataType::Int8 };
	EOperation Operation{ EOperation::Assign };
};
static_assert(sizeof(BMCCCommandHeader) == 4, "Command header is expected to be 4 bytes");