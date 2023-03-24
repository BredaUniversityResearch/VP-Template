#include "CameraControlPacketFactory.h"

#include "CameraControlDataPacket.h"
#include "CameraControlDiscoveryPacket.h"
#include "CameraControlHeartbeatPacket.h"
#include "CameraControlTimeCodeChangedPacket.h"

namespace
{
	template<typename TPacketType>
	TUniquePtr<ICameraControlPacket> ConstructorWrapper()
	{
		return MakeUnique<TPacketType>();
	}

	using PacketCtorFn = TUniquePtr<ICameraControlPacket>(*)();
	TMap<uint32, PacketCtorFn> InitializePacketTypes()
	{
		TMap<uint32, PacketCtorFn> result;
		result.Add(FCameraControlDiscoveryPacket::StaticPacketIdentifier, &ConstructorWrapper<FCameraControlDiscoveryPacket>);
		result.Add(FCameraControlDataPacket::StaticPacketIdentifier, &ConstructorWrapper<FCameraControlDataPacket>);
		result.Add(FCameraControlTimeCodeChangedPacket::StaticPacketIdentifier, &ConstructorWrapper<FCameraControlTimeCodeChangedPacket>);
		result.Add(FCameraControlHeartbeatPacket::StaticPacketIdentifier, &ConstructorWrapper<FCameraControlHeartbeatPacket>);

		return result;
	}

	TMap<uint32, PacketCtorFn> g_availablePackets = InitializePacketTypes();
}

TUniquePtr<ICameraControlPacket> FCameraControlPacketFactory::CreateFromIdentifier(int32 Identifier)
{
	PacketCtorFn* ctor = g_availablePackets.Find(Identifier);
	if (ctor != nullptr)
	{
		return (*ctor)();
	}
	return TUniquePtr<ICameraControlPacket>(nullptr);
}
