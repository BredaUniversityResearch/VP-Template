#include "DataPacket.h"

FDataPacket::IData::IData(const EPacketDataType& Type)
	:
DataType(Type)
{}

FDataPacket::FDataPacket(const FTimecode& TimeCode, const TSharedPtr<IData>& InData)
	:
GenLockTimeCode(TimeCode),
Data(InData)
{
	check(InData.IsValid())
}

void FDataPacket::Serialize(TArray<uint8>& OutData) const
{
	check(Data.IsValid())

	const auto dataTypeUTF8 = TCHAR_TO_UTF8(*Data->DataType.ToString());




}