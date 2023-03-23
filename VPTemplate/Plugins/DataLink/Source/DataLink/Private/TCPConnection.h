#pragma once

struct FIPv4Endpoint;
class FSocket;
class FInternetAddr;

class FTCPConnection
{

public:

    enum class EState
    {
        UNINITIALIZED,      //No remote address has been provide yet.
        CONNECTING,         //A remote address has been provided but no connection has been established yet.
        CONNECTING_FAILED,  //A remote address has been provided but no connection could be established within the max attempts.
        CONNECTED,          //A remote address has been provided and the connection has been established and is active.
        DISCONNECTED,       //A remote address has been provided and the connection was established but has been shutdown.
        RECONNECTING        //A remote address has been provided and the connection was established but has been lost and is now trying to be re established.
    };

    
    FTCPConnection();
    ~FTCPConnection();

    void Connect(
        const FIPv4Endpoint& RemoteEndpoint,
        const FTimespan& RetryAttemptInterval,
        uint32 MaxRetryAttempts,
        uint32 InSendBufferSize,
        uint32 InReceiveBufferSize);
    void Disconnect();
    void Update(const FTimespan& WaitTime);
    bool WaitToSend(const FTimespan& WaitTime) const;
    bool Send(const TSharedRef<TArray<uint8>>& Data);
    EState GetState() const;
    FIPv4Endpoint GetConnectionAddress() const;

private:

    void CreateSocket();

    TSharedPtr<FSocket> Socket; //Uses SharedPtr instead of UniquePtr to get thread-safe access.
    TSharedPtr<FInternetAddr> RemoteAddress; //Uses SharedPtr to get thread-safe access.

    uint32 ConnectionAttemptsCurrent;
    uint32 ConnectionAttemptsMax;

    FTimespan ConnectionAttemptInterval;

    EState State;

};