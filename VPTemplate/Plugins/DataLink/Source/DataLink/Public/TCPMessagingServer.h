#pragma once

#include "Networking.h"

class DATALINK_API FTCPMessagingServer
{
public:

	//Necessary functionalities:
	//- A way to set up a TCP connection where messages can be published to subscribers or send directly to a target. (Probably use the IMessageBus interface for this in some form.)
	//- A way to easily publish or send messages on an existing connection.
	//- A way to process requests that arrive from clients, which can be used to request data for certain time codes or start a data stream etc.
	//- Maybe functions to encode messages.

	//Initialize the server to listen at a certain (or all) Ip address(es) and a certain port for incoming connection requests.
	void Initialize(/*IpAddress = IpAddress::Any, Port*/);

private:

	bool OnConnectionAccepted(FSocket* Socket, const FIPv4Endpoint& RemoteEndPoint);

	FTcpListener Listener;

};