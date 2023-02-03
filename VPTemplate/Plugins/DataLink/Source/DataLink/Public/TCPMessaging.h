#pragma once

#include "Networking.h"

class DATALINK_API FTCPMessaging
{
public:

	FTCPMessaging();

	//Initialize the server to listen at a certain (or all) Ip address(es) and a certain port for incoming connection requests.
	//void Initialize(const FIPv4Endpoint& ListenerEndpoint);

	bool ConnectToSocket(const FIPv4Endpoint& RemoteEndpoint, int32 SendBufferSize = 1024, int32 ReceiveBufferSize = 0);
	bool Send(const TArray<uint8>& Data) const;

	

private:

	/*bool OnConnectionAccepted(FSocket* Socket, const FIPv4Endpoint& RemoteEndPoint);

	TUniquePtr<FTcpListener> Listener;*/

	TUniquePtr<FSocket> Socket;

};