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
	ensure(byteCount > 0);
	PacketData.AddDefaulted(byteCount);
	Reader.Serialize(PacketData.GetData(), byteCount);
}
