#include "TCPMessaging.h"

#include "DataPacket.h"
#include "MessageEndpoint.h"

FTCPMessaging::FTCPMessaging()
    :
Running(false)
{
    Socket = TSharedPtr<FSocket>(
        FTcpSocketBuilder("DataLinkSocketBuilder")
        .AsBlocking()
        .Build());
    RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    Thread = TUniquePtr<FRunnableThread>(FRunnableThread::Create(this, TEXT("DataLinkFTCPMessaging")));
}

FTCPMessaging::~FTCPMessaging()
{
    if(Thread.IsValid())
    {
        Thread->Kill();
    }

    if(Socket.IsValid())
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket.Get());
        Socket.Reset();
    }
}

void FTCPMessaging::ConnectToSocket(
    const FIPv4Endpoint& RemoteEndpoint,
    uint32 RetryAttempts,
    const FTimespan& RetryInterval,
    uint32 InSendBufferSize,
    uint32 InReceiveBufferSize)
{
    if(Socket.IsValid() && Socket->GetConnectionState() == USOCK_Open)
    {
        Socket->Close();
    }

    RemoteAddress->SetIp(RemoteEndpoint.Address.Value);
    RemoteAddress->SetPort(RemoteEndpoint.Port);

    ConnectionTries = 0;
    MaxConnectionTries = RetryAttempts;
    ConnectionTryInterval = RetryInterval;

}

void FTCPMessaging::Send(const TSharedRef<TArray<uint8>, ESPMode::ThreadSafe>& Data) const
{
    const auto addMessageToQueueFunction = [this, Data]()
    {
        if (Running && MessageQueue.Enqueue(Data))
        {
            UpdateEvent->Trigger();
            return true;
        }
        return false;
    };

    UE::Tasks::Launch(
        TEXT("DataLinkSendTCPMessage"),
        addMessageToQueueFunction);
}

void FTCPMessaging::Send(const FDataPacket& Packet) const
{
    const TSharedRef<TArray<uint8_t>, ESPMode::ThreadSafe> packetDataRef =
        MakeShared<TArray<uint8_t>, ESPMode::ThreadSafe>();
    Packet.Serialize(packetDataRef.Get());
    return Send(packetDataRef);
}

void FTCPMessaging::Update()
{
    if(Socket.IsValid() && Socket->GetConnectionState() != USOCK_Open)
    {

    }
}

bool FTCPMessaging::Init()
{
    Running = true;
    return Running && Socket.IsValid();
}

uint32 FTCPMessaging::Run()
{
    while(Running)
    {
        Update();
        UpdateEvent->Wait();
    }

    return 0;
}

void FTCPMessaging::Stop()
{
    Running = false;
}
