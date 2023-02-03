#pragma once
#include "PacketDataType.h"

class FDataPacket
{

public:

	class IData
	{
	public:

		IData(const EPacketDataType& Type);
		virtual ~IData() = 0;

		virtual void Serialize(TArray<uint8>& OutData) const = 0;

		const EPacketDataType DataType;

	};

	FDataPacket(const FTimecode& TimeCode, const TSharedPtr<IData>& InData);
	~FDataPacket() = default;

	//Converts the packet to JSON and outputs it as an array of bytes.
	void Serialize(TArray<uint8>& OutData) const;

private:

	void ÁddFStringToByteArray(TArray<uint8>& ByteArray) const;

	const FTimecode GenLockTimeCode;
	const TSharedPtr<IData> Data;
 
};