#pragma once

enum class ECommandId : uint8_t
{
	ChangeConfig,
};

struct FBMCCPacketHeader
{
	uint8_t TargetCamera{ 255 };//255 is broadcast
	uint8_t PacketSize{ 0 };
	ECommandId CommandId{ ECommandId::ChangeConfig };
	uint8_t Reserved{ 0 };
};
static_assert(sizeof(FBMCCPacketHeader) == 4, "Packet header is expected to be 4 bytes");
