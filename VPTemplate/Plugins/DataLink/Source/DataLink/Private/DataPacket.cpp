#include "DataPacket.h"


FDataPacket::FDataPacket(const FTimecode& TimeCode, const TSharedPtr<IData>& InData)
	:
GenLockTimeCode(TimeCode),
Data(InData)
{
	check(InData.IsValid())
}

void FDataPacket::Serialize(TArray<uint8>& OutData) const
{
	const TSharedPtr<FJsonObject> jsonDataPacket = MakeShareable(new FJsonObject());
	jsonDataPacket->SetStringField("DataType", Data.IsValid() ? Data->DataType.ToString() : "None");
	jsonDataPacket->SetStringField("TimeCode", GenLockTimeCode.ToString());

	if(Data.IsValid())
	{
		const TSharedPtr<FJsonObject> jsonDataObject = MakeShareable(new FJsonObject());
		Data->Serialize(jsonDataObject);
		jsonDataPacket->SetObjectField("Data", jsonDataObject);
	}

	FString jsonDataString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonDataString);
	FJsonSerializer::Serialize(jsonDataPacket.ToSharedRef(), Writer);

	StringToByteArrayAsUTF8(jsonDataString, OutData);
}

void FDataPacket::StringToByteArrayAsUTF8(const FString& String, TArray<uint8>& ByteArray)
{
	if(String.IsEmpty())
	{
		return;
	}

	const int32 stringLengthWithNull = String.Len() + 1;

	//Why not UTF8CHAR instead of ANSICHAR?   idea but this is what Unreal provides :(
	const TArray<ANSICHAR> stringAsUtf8 (TCHAR_TO_UTF8(*String), stringLengthWithNull);
	for(const auto& character : stringAsUtf8)
	{
		ByteArray.Add(static_cast<uint8>(character));
	}
}



FDataPacket::IData::IData(const EPacketDataType& Type)
	:
	DataType(Type)
{}

FDataPacket::IData::~IData()
{}



FDataPacket::FNoData::FNoData()
	:
IData(EPacketDataType::NONE)
{}

FDataPacket::FNoData::~FNoData()
{}

void FDataPacket::FNoData::Serialize(TSharedPtr<FJsonObject> JsonDataObject) const
{}
