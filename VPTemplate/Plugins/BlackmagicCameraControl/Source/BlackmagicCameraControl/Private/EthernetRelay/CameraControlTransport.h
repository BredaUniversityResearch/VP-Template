#pragma once

class ICameraControlPacket;

class FCameraControlTransport
{
public:
	template<typename TPacketType>
	static void Write(const TPacketType& a_packet, FMemoryWriter& a_writer);
	static TUniquePtr<ICameraControlPacket> TryRead(FMemoryReader& Reader);
	static FString SerializeString(FMemoryArchive& Reader);

private:
	static void Write(uint32 PacketIdentifier, const ICameraControlPacket& a_packet, FMemoryWriter& a_writer);
};

template <typename TPacketType>
void FCameraControlTransport::Write(const TPacketType& a_packet, FMemoryWriter& a_writer)
{
	Write(TPacketType::StaticPacketIdentifier, a_packet, a_writer);
}
