#include "CameraControlDataPacket.h"

#include "CameraControlTransport.h"

void FCameraControlDataPacket::WriteTo(FMemoryWriter& Writer)
{
	checkNoEntry();
}

void FCameraControlDataPacket::ReadFrom(FMemoryReader& Reader)
{
	DeviceUuid = FCameraControlTransport::SerializeString(Reader);

	Reader << ReceivedTimeCodeAsBcd;
	uint16 byteCount = 0;
	Reader << byteCount;
	PacketData.Reserve(byteCount);
	for (int i = 0; i < byteCount; ++i)
	{
		uint8 byte;
		Reader << byte;
		PacketData.Emplace(byte);
	}
}
