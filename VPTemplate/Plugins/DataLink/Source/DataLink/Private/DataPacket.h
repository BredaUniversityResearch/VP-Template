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

		virtual void Serialize(TSharedPtr<FJsonObject> JsonDataObject) const = 0;

		const EPacketDataType DataType;
	};

	class FNoData final : public IData
	{
	public:

		FNoData();
		virtual ~FNoData() override;

		virtual void Serialize(TSharedPtr<FJsonObject> JsonDataObject) const override;
	};

	FDataPacket(const FTimecode& TimeCode, const TSharedPtr<IData>& InData);
	~FDataPacket() = default;

	//Converts the packet to JSON and outputs it encoded as UTF8 as an array of bytes.
	void Serialize(TArray<uint8>& OutData) const;

private:
	
	static void StringToByteArrayAsUTF8(const FString& String, TArray<uint8>& ByteArray);

	const FTimecode GenLockTimeCode;
	const TSharedPtr<IData> Data;
 
};