#pragma once
#include "Common/TcpListener.h"

class IBMCCDataReceivedHandler;

class FCameraControlNetworkReceiver
{
public:
	struct FConnectedTransmitter
	{
		FSocket* ClientSocket{ nullptr };
		FDateTime LastConnectionTime;
		TUniquePtr<FRunnable> ReceiveThread;
	};

	explicit FCameraControlNetworkReceiver(IBMCCDataReceivedHandler* DataReceivedHandler);
	~FCameraControlNetworkReceiver();

	void Start();
	void Stop();
	void Update();

private:
	bool OnConnectionAccepted(FSocket* ConnectionSocket, const FIPv4Endpoint& RemoteEndpoint);

	IBMCCDataReceivedHandler* m_dataHandler;
	TUniquePtr<FRunnable> m_discoveryBroadcastTask;
	TUniquePtr<FTcpListener> m_connectionListener;
	TArray<TUniquePtr<FConnectedTransmitter>> m_activeConnections;
};
