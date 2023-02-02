#include "TCPMessagingServer.h"

#include "MessageEndpoint.h"
#include "MessageEndpointBuilder.h"

void FTCPMessagingServer::Initialize()
{

	Listener = FTcpListener(FIPv4Endpoint::Any);
	Listener.OnConnectionAccepted().BindRaw(this, &FTCPMessagingServer::OnConnectionAccepted);
	Listener.Init();

}

bool FTCPMessagingServer::OnConnectionAccepted(FSocket* Socket, const FIPv4Endpoint& RemoteEndPoint)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, FString::Format(TEXT("Connection Accepted from endpoint: {0}"), FStringFormatOrderedArguments{ {RemoteEndPoint.ToString()} }));
	}
	return true;
}