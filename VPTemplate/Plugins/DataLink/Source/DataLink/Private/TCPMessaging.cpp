#include "TCPMessaging.h"

#include "DataPacket.h"
#include "MessageEndpoint.h"

FTCPMessaging::FTCPMessaging()
    :
Running(false),
WaitTime(FTimespan::FromMilliseconds(100.0))
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
        Thread->Kill(true);
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
    const auto connectFunction = 
        [this, 
        RemoteEndpoint,
        RetryAttempts,
        RetryInterval,
        InSendBufferSize, 
        InReceiveBufferSize]()
    {
        if (Socket.IsValid() && Socket->GetConnectionState() == USOCK_Open)
        {
            Socket->Close();
        }

        RemoteAddress->SetIp(RemoteEndpoint.Address.Value);
        RemoteAddress->SetPort(RemoteEndpoint.Port);

        int32 bufferSize = 0;
        Socket->SetSendBufferSize(InSendBufferSize, bufferSize);
        Socket->SetReceiveBufferSize(InReceiveBufferSize, bufferSize);

        ConnectionTries.Store(0);
        MaxConnectionTries.Store(RetryAttempts);
        ConnectionTryInterval.Store(RetryInterval);

        UpdateEvent->Trigger();
    };
    
    UE::Tasks::Launch(
        TEXT("DataLinkConnectTCPSocket"),
        connectFunction);

}

void FTCPMessaging::Send(const TSharedRef<TArray<uint8>>& Data)
{
    const auto sendFunction = [this, Data]()
    {
        if (Running && MessageQueue.Enqueue(FTCPMessage{ Data }))
        {
            UpdateEvent->Trigger();
            return true;
        }
        return false;
    };

    UE::Tasks::Launch(
        TEXT("DataLinkSendTCPMessage"),
        sendFunction);
}

void FTCPMessaging::Send(const FDataPacket& Packet)
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
        while(Socket->GetConnectionState() != USOCK_Open && (ConnectionTries+1) <= MaxConnectionTries)
        {
            ++ConnectionTries;
            if(!Socket->Connect(*RemoteAddress))
            {
                FPlatformProcess::Sleep(ConnectionTryInterval.Load().GetTotalSeconds());
            }
        }
        ConnectionTries.Store(0);
        MaxConnectionTries.Store(UINT32_MAX);
    }
    else if(Socket.IsValid() && Socket->GetConnectionState() == USOCK_Open)
    {
        while(!MessageQueue.IsEmpty())
        {
            if(!Socket->Wait(ESocketWaitConditions::WaitForWrite, WaitTime))
            {
                break;
            }

            FTCPMessage message;
            MessageQueue.Dequeue(message);

            int32 sent = 0;
            Socket->Send(message.Data->GetData(), message.Data->Num(), sent);

            //TODO: handle case where not all data is sent.
        }
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
        UpdateEvent->Wait(WaitTime);
    }

    return 0;
}

void FTCPMessaging::Stop()
{
    Running = false;
}
