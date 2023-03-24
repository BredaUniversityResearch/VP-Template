#pragma once
#include "ICameraControlPacket.h"

class FCameraControlTimeCodeChangedPacket : public ICameraControlPacket
{
public:
	static inline const uint32 StaticPacketIdentifier = DBJ2aHash(L"CameraControlTimeCodeChanged");

	virtual void WriteTo(FMemoryWriter& Writer) override;
	virtual void ReadFrom(FMemoryReader& Reader) override;
	virtual uint32 GetPacketType() const override { return StaticPacketIdentifier; }

	FString DeviceUuid;
	uint32 TimeCodeAsBCD;
};