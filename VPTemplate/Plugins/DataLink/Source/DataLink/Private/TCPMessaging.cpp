#include "TCPMessaging.h"

#include "DataPacket.h"
#include "MessageEndpoint.h"
#include "TCPConnection.h"

FTCPMessaging::FTCPMessaging()
    :
Connection(MakeShared<FTCPConnection>()),
Running(false),
WaitTime(FTimespan::FromMilliseconds(100.0))
{
    Thread = TUniquePtr<FRunnableThread>(FRunnableThread::Create(this, TEXT("DataLink.FTCPMessaging")));
}

FTCPMessaging::~FTCPMessaging()
{
    if(Thread.IsValid())
    {
        Thread->Kill(true);
    }
}

void FTCPMessaging::ConnectSocket(
    const FIPv4Endpoint& RemoteEndpoint,
    const FTimespan& RetryInterval,
    uint32 MaxRetryAttempts,
    uint32 SendBufferSize,
    uint32 ReceiveBufferSize) const
{
    //Uses Tasks System to avoid blocking calls as accessing Connection can block.
    //TODO: check if Connection can actually block when accessing and if it even provides thread-safe access.
    const auto taskName = TEXT("DataLinkConnectTCPSocket");
    const auto connectFunction = 
        [this, 
        RemoteEndpoint,
        RetryInterval,
        MaxRetryAttempts,
        SendBufferSize, 
        ReceiveBufferSize]()
    {
        Connection->Connect(
            RemoteEndpoint,
            RetryInterval,
            MaxRetryAttempts,
            SendBufferSize,
            ReceiveBufferSize);
        UpdateEvent->Trigger();
    };
    
    UE::Tasks::Launch(taskName, connectFunction);
}

bool FTCPMessaging::Send(const TSharedRef<TArray<uint8>>& Data)
{
    if (Running && MessageQueue.Enqueue(Data))
    {
        UpdateEvent->Trigger();
        return true;
    }
    return false;
}

bool FTCPMessaging::Send(const FDataPacket& Packet)
{
    const auto packetDataRef = MakeShared<TArray<uint8_t>, ESPMode::ThreadSafe>();
    Packet.Serialize(packetDataRef.Get());
    return Send(packetDataRef);
}

void FTCPMessaging::Update()
{
    check(Connection.IsValid());

    Connection->Update(WaitTime);
    while(!MessageQueue.IsEmpty() && Connection->GetState() == FTCPConnection::EState::CONNECTED)
    {
        if(!Connection->WaitToSend(WaitTime))
        {
            break;
        }

        //Notice: only remove message from queue if it could actually be sent.
        //It might happen that the Connection->Update doesn't detect disconnection
        //from the remote, due to an known issue with FSocket->GetConnectionState(),
        //and then fails to set the right state. The work around is to detect if data
        //can not be sent, but instead of trying to send an dummy this solution simply
        //tries to send the message from the queue and only pops it from the queue if
        //it could actually be sent.
        FTCPMessage message;
        if(MessageQueue.Peek(message))
        {
            if(Connection->Send(message.ToSharedRef()))
            {
                MessageQueue.Pop();
            }
            else
            {
                break;
                //Get out of the while loop if message could not be sent as it
                //most likely means that the connection with the remote disconnected.
            }
        }
        
    }
}

bool FTCPMessaging::Init()
{
    Running = true;
    return Running;
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
    UpdateEvent->Trigger();
}
