#include "TCPMessaging.h"

#include "DataPacket.h"
#include "MessageEndpoint.h"
#include "MessageEndpointBuilder.h"

FTCPMessaging::FTCPMessaging()
    /*:
Listener(nullptr)*/
{}

//void FTCPMessaging::Initialize(const FIPv4Endpoint& EndPoint)
//{
//
//	Listener = MakeUnique<FTcpListener>(EndPoint);
//	Listener->OnConnectionAccepted().BindRaw(this, &FTCPMessaging::OnConnectionAccepted);
//	Listener->Init();
//
//}
//
//bool FTCPMessaging::OnConnectionAccepted(FSocket* Socket, const FIPv4Endpoint& RemoteEndPoint)
//{
//	if(GEngine)
//	{
//		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, FString::Format(TEXT("Connection Accepted from endpoint: {0}"), FStringFormatOrderedArguments{ {RemoteEndPoint.ToString()} }));
//	}
//
//	Socket.
//
//	return true;
//}

bool FTCPMessaging::ConnectToSocket(
    const FIPv4Endpoint& RemoteEndpoint, 
    int32 SendBufferSize,
    int32 ReceiveBufferSize)
{
    if(Socket.IsValid())
    {
        Socket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket.Get());
    }

    FTcpSocketBuilder socketBuilder("DataLinkFTcpMessaging");
    socketBuilder.AsBlocking();
    socketBuilder.WithSendBufferSize(FMath::Max(SendBufferSize, 0));
    socketBuilder.WithReceiveBufferSize(FMath::Max(ReceiveBufferSize, 0));

    if(Socket.IsValid())
    {
        Socket.Reset(socketBuilder.Build());
    }
    else
    {
        Socket = TUniquePtr<FSocket>(socketBuilder.Build());
    }

    if(Socket)
    {
        const TSharedRef<FInternetAddr> remoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
        remoteAddress->SetIp(RemoteEndpoint.Address.Value);
        remoteAddress->SetPort(RemoteEndpoint.Port);

        return Socket->Connect(*remoteAddress);
    }
    return false;
}

bool FTCPMessaging::Send(const TArray<uint8>& Data) const
{
	if(Socket)
	{
        int32 sentSize = 0;
        return Socket->Send(Data.GetData(), Data.Num(), sentSize);
	}
    return false;
}

bool FTCPMessaging::Send(const FDataPacket& Packet) const
{
    if(Socket)
    {
        TArray<uint8> packetData;
        Packet.Serialize(packetData);

        int32 sentSize = 0;
        return Socket->Send(packetData.GetData(), packetData.Num(), sentSize);
    }
    return false;
}