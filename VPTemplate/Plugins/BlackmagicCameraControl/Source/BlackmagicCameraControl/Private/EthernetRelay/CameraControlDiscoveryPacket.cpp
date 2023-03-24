#include "CameraControlDiscoveryPacket.h"

void FCameraControlDiscoveryPacket::WriteTo(FMemoryWriter& Writer)
{
	Writer << MagicBits;
	Writer << ServerIdentifier;
	Writer << TargetPort;
}

void FCameraControlDiscoveryPacket::ReadFrom(FMemoryReader& Reader)
{
	Reader << MagicBits;
	Reader << ServerIdentifier;
	Reader << TargetPort;
}
