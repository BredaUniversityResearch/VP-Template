#include "CameraControlNetworkReceiver.h"

#include "BMCCDataReceivedHandler.h"
#include "BMCCTransportProtocol.h"
#include "CameraControlDataPacket.h"
#include "CameraControlDiscoveryPacket.h"
#include "CameraControlTransport.h"
#include "Common/UdpSocketBuilder.h"
#include "BlackmagicCameraControl.h"

namespace
{
	static const FIPv4Address DiscoveryMulticastAddress = FIPv4Address(224, 0, 0, 69);
	static constexpr uint16 DiscoveryMulticastPort = 49069;
	static const FIPv4Endpoint DiscoveryMulticastEndpoint = FIPv4Endpoint(DiscoveryMulticastAddress, DiscoveryMulticastPort);
	static constexpr float DiscoveryMulticastIntervalSeconds = 5.0f;
	static constexpr uint16 ConnectionPortRangeStart = 49070;
	static constexpr uint16 ConnectionPortRangeEnd = ConnectionPortRangeStart + 32;
	static const FTimespan InactivityDisconnectTime = FTimespan::FromSeconds(15.0);

	class FBackgroundDiscoveryBroadcasterRunnable : public FRunnable
	{
	public:
		FBackgroundDiscoveryBroadcasterRunnable(FCameraControlNetworkReceiver* Owner, int ListeningPort)
			: m_owner(Owner)
			, m_discoveryBroadcaster(nullptr)
			, m_listeningPort(ListeningPort)
		{
			m_thread = FRunnableThread::Create(this, TEXT("CameraControlDiscoveryBroadcaster"));
		}

		virtual ~FBackgroundDiscoveryBroadcasterRunnable() override
		{
			ISocketSubsystem::Get()->DestroySocket(m_discoveryBroadcaster);
		}

		virtual bool Init() override
		{
			//Get any kind of random int32. FMath::Rand isn't properly random seeded at this point.
			m_serverIdentifier = static_cast<int32>(FGuid::NewGuid().B);
			m_discoveryBroadcaster = FUdpSocketBuilder("CameraControlDiscoveryBroadcaster")
				.AsBlocking()
				.WithMulticastLoopback()
				.WithMulticastTtl(1)
				.Build();
			if (m_discoveryBroadcaster == nullptr)
			{
				UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Failed to setup broadcasting socket"));
			}

			return m_discoveryBroadcaster != nullptr;
		}

		virtual uint32 Run() override
		{
			TSharedRef<FInternetAddr> multicastEndpoint = DiscoveryMulticastEndpoint.ToInternetAddr();
			TArray<uint8, TSizedDefaultAllocator<32>> buffer;
			while (!m_stopRequested)
			{
				FMemoryWriter writer = FMemoryWriter(buffer);
				FCameraControlTransport::Write(FCameraControlDiscoveryPacket(m_serverIdentifier, m_listeningPort), writer);

				ensureAlways(writer.Tell() < TNumericLimits<int32>::Max());

				int32 bytesSent = 0;
				bool result = m_discoveryBroadcaster->SendTo(buffer.GetData(), writer.Tell(), bytesSent, *multicastEndpoint);
				if (bytesSent != writer.Tell() || !result)
				{
					UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Failed to send discovery broadcast. Expected to send %i, actually sent %i, result: %i, ErrorCode %s"), 
						static_cast<int32>(writer.Tell()), bytesSent, result, ISocketSubsystem::Get()->GetSocketError());
				}
				FPlatformProcess::Sleep(DiscoveryMulticastIntervalSeconds);
			}
			return 0;
		}

		virtual void Stop() override
		{
			m_stopRequested = true;
			m_thread->WaitForCompletion();
		}

	private:
		FCameraControlNetworkReceiver* m_owner;
		int32 m_serverIdentifier{};
		bool m_stopRequested{ false };
		FSocket* m_discoveryBroadcaster{ nullptr };
		int32 m_listeningPort{};
		FRunnableThread* m_thread;
	};

	class FBackgroundReceiveThread : public FRunnable
	{
	public:
		FBackgroundReceiveThread(FCameraControlNetworkReceiver::FConnectedTransmitter* transmitter, IBMCCDataReceivedHandler* Dispatcher)
			: m_transmitter(transmitter)
			, m_dispatcher(Dispatcher)
		{
			m_runningThread = FRunnableThread::Create(this, TEXT("CameraControlReceiveThread"));
		}

		virtual uint32 Run() override
		{
			uint8 buffer[8192];
			while (m_transmitter->ClientSocket->GetConnectionState() == SCS_Connected)
			{
				int32 bytesRead = 0;
				bool success = m_transmitter->ClientSocket->Recv(buffer, sizeof(buffer), bytesRead);
				if (success && bytesRead > 0)
				{
					TArray<uint8> memoryBuffer = TArray<uint8>(buffer, bytesRead);
					FMemoryReader reader = FMemoryReader(memoryBuffer);
					while (reader.Tell() < bytesRead)
					{
						TUniquePtr<ICameraControlPacket> packet = FCameraControlTransport::TryRead(reader);
						if (packet != nullptr)
						{
							if (packet->GetPacketType() == FCameraControlDataPacket::StaticPacketIdentifier)
							{
								FCameraControlDataPacket* dataPacket = static_cast<FCameraControlDataPacket*>(packet.Get());
								BMCCDeviceHandle handle = BMCCDeviceHandle_Broadcast;
								BMCCTransportProtocol::DecodeStream(dataPacket->PacketData, handle, m_dispatcher);
							}
						}
						else
						{
							UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Discarded %i/%i bytes of data due to missing packet types"), bytesRead - static_cast<int>(reader.Tell()), bytesRead);
							break;
						}

						m_transmitter->LastConnectionTime = FDateTime::UtcNow();
					}
				}
			}
			return 0;
		}

		void WaitForCompletion() const
		{
			m_runningThread->WaitForCompletion();
		}

	private:
		FRunnableThread* m_runningThread{ nullptr };
		FCameraControlNetworkReceiver::FConnectedTransmitter* m_transmitter;
		IBMCCDataReceivedHandler* m_dispatcher;
	};
}

void FCameraControlNetworkReceiver::FConnectedTransmitter::Close() 
{
	ClientSocket->Close();
	static_cast<FBackgroundReceiveThread*>(ReceiveThread.Get())->WaitForCompletion();
	ClientSocket = nullptr;
}

FCameraControlNetworkReceiver::FCameraControlNetworkReceiver(IBMCCDataReceivedHandler* DataReceivedHandler)
	: m_dataHandler(DataReceivedHandler)
{
}

FCameraControlNetworkReceiver::~FCameraControlNetworkReceiver()
{
	Stop();
}

void FCameraControlNetworkReceiver::Start()
{

	for (uint16 connectPortToTry = ConnectionPortRangeStart; connectPortToTry < ConnectionPortRangeEnd; ++connectPortToTry)
	{
		UE_LOG(LogBlackmagicCameraControl, Log, TEXT("Attempting to start network receiver on port %u"), connectPortToTry);
		m_connectionListener = MakeUnique<FTcpListener>(FIPv4Endpoint(FIPv4Address::Any, connectPortToTry));
		if (m_connectionListener->IsActive())
		{
			UE_LOG(LogBlackmagicCameraControl, Log, TEXT("Success... Listening for connections on port %u"), connectPortToTry);
			break;
		}
	}

	if (!m_connectionListener->IsActive())
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Failed to create a listener, exhausted entire port range from %u-%u"), ConnectionPortRangeStart, ConnectionPortRangeEnd);
	}
	m_connectionListener->OnConnectionAccepted().BindRaw(this, &FCameraControlNetworkReceiver::OnConnectionAccepted);

	m_discoveryBroadcastTask = MakeUnique<FBackgroundDiscoveryBroadcasterRunnable>(this, m_connectionListener->GetLocalEndpoint().Port);
}

void FCameraControlNetworkReceiver::Stop()
{
	for (const TUniquePtr<FConnectedTransmitter>& connected : m_activeConnections)
	{
		connected->Close();
	}
	m_activeConnections.Empty();

	if (m_discoveryBroadcastTask != nullptr)
	{
		m_discoveryBroadcastTask->Stop();
		m_discoveryBroadcastTask.Reset();
	}
}

void FCameraControlNetworkReceiver::Update()
{
	FDateTime now = FDateTime::UtcNow();
	for (int i = m_activeConnections.Num() - 1; i >= 0; --i)
	{
		if ((now - m_activeConnections[i]->LastConnectionTime).GetDuration() > InactivityDisconnectTime ||
			m_activeConnections[i]->ClientSocket->GetConnectionState() != SCS_Connected)
		{
			UE_LOG(LogBlackmagicCameraControl, Log, TEXT("Connection closed from server"));
			m_activeConnections[i]->Close();
			m_activeConnections.RemoveAt(i);

		}
	}
}

bool FCameraControlNetworkReceiver::OnConnectionAccepted(FSocket* ConnectionSocket, const FIPv4Endpoint& RemoteEndpoint)
{
	TUniquePtr<FConnectedTransmitter> connection = MakeUnique<FConnectedTransmitter>();
	connection->ClientSocket = ConnectionSocket;
	connection->LastConnectionTime = FDateTime::UtcNow();
	connection->ReceiveThread = MakeUnique<FBackgroundReceiveThread>(connection.Get(), m_dataHandler);
	m_activeConnections.Emplace(MoveTemp(connection));

	UE_LOG(LogBlackmagicCameraControl, Log, TEXT("Connection accepted from %s"), *RemoteEndpoint.ToString());

	return true;
}
