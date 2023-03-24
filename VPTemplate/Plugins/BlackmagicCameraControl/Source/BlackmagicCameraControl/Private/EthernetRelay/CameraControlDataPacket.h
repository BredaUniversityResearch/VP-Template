#pragma once
#include "ICameraControlPacket.h"

class FCameraControlDataPacket: public ICameraControlPacket
{
public:
	static inline const uint32 StaticPacketIdentifier = DBJ2aHash(L"CameraControlDataPacket");

	virtual void WriteTo(FMemoryWriter& Writer) override;
	virtual void ReadFrom(FMemoryReader& Reader) override;
	virtual uint32 GetPacketType() const override { return StaticPacketIdentifier; }

	FString DeviceUuid;
	uint32 ReceivedTimeCodeAsBcd;
	TArray<uint8> PacketData;
};