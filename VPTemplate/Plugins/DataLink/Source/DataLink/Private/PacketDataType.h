#pragma once

//Disclaimer: Trick learned from Jens:
//https://stackoverflow.com/questions/21295935/can-a-c-enum-class-have-methods
class EPacketDataType
{
public:
	enum Value : uint8
	{
		NONE,
		TRACKERDATA
	};

	EPacketDataType() = default;
	constexpr EPacketDataType(Value v) : Value(v) {}

	constexpr operator Value() const { return Value; }

	explicit operator bool() const = delete;

	constexpr bool operator== (Value& v) const { return v == Value; }
	constexpr bool operator!= (Value& v) const { return v != Value; }

	FString ToString() const
	{
		switch (Value)
		{
		case NONE:
			return "None";
		case TRACKERDATA:
			return "TrackerData";
		default:
			return "";
		}
	}

private:
	Value Value;
};