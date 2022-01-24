#pragma once

struct FBMCCCommandIdentifier
{
	constexpr FBMCCCommandIdentifier() = default;
	constexpr FBMCCCommandIdentifier(uint8 Category, uint8 Parameter)
		: Category(Category)
		, Parameter(Parameter)
	{
	}

	bool operator==(const FBMCCCommandIdentifier& rhs) const
	{
		return Category == rhs.Category && Parameter == rhs.Parameter;
	}

	uint8 Category{ 255 };
	uint8 Parameter{ 255 };
};
