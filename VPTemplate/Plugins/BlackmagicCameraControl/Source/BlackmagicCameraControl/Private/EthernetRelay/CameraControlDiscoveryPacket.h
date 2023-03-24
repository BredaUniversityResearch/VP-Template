#pragma once
#include "ICameraControlPacket.h"

class FCameraControlDiscoveryPacket: public ICameraControlPacket
{
public:
	static inline const uint32 StaticPacketIdentifier = DBJ2aHash(L"CameraControlDiscoveryPacket");

	static constexpr uint32 ExpectedMagicBits = 0xA0C0FFEE;
	uint32 MagicBits = 0;
	int ServerIdentifier{};
	int TargetPort{};

	FCameraControlDiscoveryPacket() = default;

	FCameraControlDiscoveryPacket(int a_serverIdentifier, int a_serverPort)
		: MagicBits(ExpectedMagicBits)
		, ServerIdentifier(a_serverIdentifier)
		, TargetPort(a_serverPort)
	{
	}

	virtual void WriteTo(FMemoryWriter& Writer) override;
	virtual void ReadFrom(FMemoryReader& Reader) override;
	virtual uint32 GetPacketType() const override { return StaticPacketIdentifier; }
};
