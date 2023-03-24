#include "CameraControlTimeCodeChangedPacket.h"

#include "CameraControlTransport.h"

void FCameraControlTimeCodeChangedPacket::WriteTo(FMemoryWriter& Writer)
{
	checkNoEntry();
}

void FCameraControlTimeCodeChangedPacket::ReadFrom(FMemoryReader& Reader)
{
	DeviceUuid = FCameraControlTransport::SerializeString(Reader);
	Reader << TimeCodeAsBCD;
}
