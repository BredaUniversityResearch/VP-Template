#pragma once

class ICameraControlPacket
{
public:
	virtual ~ICameraControlPacket() = default;

	virtual void WriteTo(FMemoryWriter& Writer) = 0;
	virtual void ReadFrom(FMemoryReader& Reader) = 0;
	virtual uint32 GetPacketType() const = 0;

protected:
	static constexpr uint32 DBJ2aHash(FStringView a_string)
	{
		uint32 hash = 5381;
		for (int index = 0; index < a_string.Len(); index++)
		{
			// Equivalent to: `hash * 33 ^ a_data[index]`
			hash = ((hash << 5) + hash) ^ a_string[index];
		}

		return hash;
	}
};