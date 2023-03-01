#include "TCPConnection.h"
#include "Networking.h"

FTCPConnection::FTCPConnection()
    :
RemoteAddress(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr()),
ConnectionAttemptsCurrent(0),
ConnectionAttemptsMax(0),
ConnectionAttemptInterval(FTimespan::Zero()),
State(EState::UNINITIALIZED)
{
    CreateSocket();
}

FTCPConnection::~FTCPConnection()
{
    if (Socket.IsValid())
    {
        Socket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket.Get());
        Socket.Reset();
    }
}

void FTCPConnection::Connect(
    const FIPv4Endpoint& RemoteEndpoint,
    const FTimespan& RetryAttemptInterval,
    uint32 MaxRetryAttempts,
    uint32 InSendBufferSize,
    uint32 InReceiveBufferSize)
{
    check(Socket.IsValid())
    check(RemoteAddress.IsValid())

    //If the socket is already connected.
    if (Socket->GetConnectionState() == SCS_Connected)
    {
        CreateSocket();
    }

    RemoteAddress->SetIp(RemoteEndpoint.Address.Value);
    RemoteAddress->SetPort(RemoteEndpoint.Port);

    int32 sendBufferSize = 0;
    if (!Socket->SetSendBufferSize(InSendBufferSize, sendBufferSize))
    {
        GEngine->AddOnScreenDebugMessage(567234, 10.f, FColor::Red,
            FString::Format(
                TEXT("Could not set socket send buffer size to desired size, desired:{0}, set:{1}"),
                FStringFormatOrderedArguments{ InSendBufferSize, sendBufferSize }));
    }
    int32 receiveBufferSize = 0;
    if (!Socket->SetReceiveBufferSize(InReceiveBufferSize, receiveBufferSize))
    {
        GEngine->AddOnScreenDebugMessage(567234, 10.f, FColor::Red,
            FString::Format(
                TEXT("Could not set socket receive buffer size to desired size, desired:{0}, set:{1}"),
                FStringFormatOrderedArguments{ InReceiveBufferSize, receiveBufferSize }));
    }

    ConnectionAttemptsCurrent = 0;
    ConnectionAttemptsMax = MaxRetryAttempts;
    ConnectionAttemptInterval = RetryAttemptInterval;

    State = EState::CONNECTING;
    Socket->Connect(*RemoteAddress);
}

void FTCPConnection::Disconnect()
{
    State = EState::DISCONNECTED;
    if (Socket->GetConnectionState() == SCS_Connected)
    {
        CreateSocket();
    }
}

void FTCPConnection::Update(const FTimespan& WaitTime)
{
    const bool ZeroSleep = WaitTime == FTimespan::Zero();
    const ESocketConnectionState socketState = Socket->GetConnectionState();

    switch (State)
    {
    case EState::UNINITIALIZED:
    case EState::CONNECTING_FAILED:
    case EState::DISCONNECTED:
    default:
        break;

    case EState::CONNECTING:
        {
            
            if (socketState == SCS_Connected)
            {
                State = EState::CONNECTED;
            }
            else
            {
                if (ConnectionAttemptsCurrent < ConnectionAttemptsMax)
                {
                    ++ConnectionAttemptsCurrent;
                    FPlatformProcess::Sleep(WaitTime.GetTotalSeconds());
                    Socket->Connect(*RemoteAddress);
                }
                else
                {
                    State = EState::CONNECTING_FAILED;
                }
            }
        }
        break;

    case EState::CONNECTED:
        {
            if(socketState != SCS_Connected)
            {
                State = EState::RECONNECTING;
                //Recreate the socket so the GetConnectionState wont return SCS_Connected after this without actually connecting again.
                CreateSocket();
                Socket->Connect(*RemoteAddress);
            }
        }
        break;

    case EState::RECONNECTING:
        {
            if(socketState == SCS_Connected)
            {
                State = EState::CONNECTED;
            }
            else
            {
                FPlatformProcess::Sleep(WaitTime.GetTotalSeconds());
                Socket->Connect(*RemoteAddress);
            }
        }
        break;

    }
}

bool FTCPConnection::WaitToSend(const FTimespan& WaitTime) const
{
    return Socket->Wait(ESocketWaitConditions::WaitForWrite, WaitTime);
}

bool FTCPConnection::Send(const TSharedRef<TArray<uint8>>& Data)
{
    int32 sentData = 0;
    Socket->Send(Data->GetData(), Data->Num(), sentData);
    const bool sentAllData = sentData == Data->Num();

    //Notice: Workaround to detect disconnection from remote
    //as GetConnectionState still returns SCS_CONNECTED when disconnected.
    //Issue is known by Epic Games but fix hasn't been accepted:
    //https://forums.unrealengine.com/t/4-12-5-fsocket-getconnectionstate-returns-incorrect-value-when-disconnected/363822/17
    //https://github.com/EpicGames/UnrealEngine/pull/2882
    if(!sentAllData && sentData == -1 && State == EState::CONNECTED)
    {
        State = EState::RECONNECTING;
        //Recreate the socket so the GetConnectionState wont return SCS_Connected after this without actually connecting again.
        CreateSocket();
    }

    return sentAllData;
}

FTCPConnection::EState FTCPConnection::GetState() const
{
    return State;
}

void FTCPConnection::CreateSocket()
{
    if(Socket.IsValid())
    {
        if (!Socket->Close())
        {
            GEngine->AddOnScreenDebugMessage(456123, 10.f, FColor::Red, "Error occurred trying to close open socket");
            return;
        }
        Socket.Reset();
    }

    Socket = MakeShareable(
        FTcpSocketBuilder("DataLink.FTCPSocket")
        .AsBlocking()
        .Build(), 
        [](FSocket* InSocket)
        {
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(InSocket);
        });
}
