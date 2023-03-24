#pragma once
#include "ICameraControlPacket.h"

class FCameraControlHeartbeatPacket : public ICameraControlPacket
{
public:
	static inline const uint32 StaticPacketIdentifier = DBJ2aHash(L"CameraControlHeartbeat");

	virtual void WriteTo(FMemoryWriter& Writer) override;
	virtual void ReadFrom(FMemoryReader& Reader) override;
	virtual uint32 GetPacketType() const override { return StaticPacketIdentifier; }
};
