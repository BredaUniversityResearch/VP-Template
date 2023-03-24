#include "CameraControlTransport.h"

#include "BlackmagicCameraControl.h"
#include "ICameraControlPacket.h"
#include "CameraControlPacketFactory.h"

TUniquePtr<ICameraControlPacket> FCameraControlTransport::TryRead(FMemoryReader& Reader)
{
	uint32 packetIdentifier;
	Reader << packetIdentifier;
	TUniquePtr<ICameraControlPacket> packet = FCameraControlPacketFactory::CreateFromIdentifier(packetIdentifier);
	if (packet != nullptr)
	{
		packet->ReadFrom(Reader);
	}
	else
	{
		UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Failed to create packet for identifier %u "), packetIdentifier);
	}
	return packet;
}

FString FCameraControlTransport::SerializeString(FMemoryArchive& Reader)
{
	uint16 strLen;
	Reader << strLen;
	TUniquePtr<char[]> buffer = MakeUnique<char[]>(strLen);
	Reader.Serialize(buffer.Get(), strLen);
	return ANSI_TO_TCHAR(buffer.Get());;
}

void FCameraControlTransport::Write(uint32 PacketIdentifier, const ICameraControlPacket& a_packet, FMemoryWriter& a_writer)
{
	a_writer << PacketIdentifier;
	//I really don't want to, but the FMemoryWriter needs non-const references to write. 
	const_cast<ICameraControlPacket&>(a_packet).WriteTo(a_writer); 
}
