#pragma once
#include "ICameraControlPacket.h"

class FCameraControlPacketFactory
{
public:
	static TUniquePtr<ICameraControlPacket> CreateFromIdentifier(int32 Identifier);
};
