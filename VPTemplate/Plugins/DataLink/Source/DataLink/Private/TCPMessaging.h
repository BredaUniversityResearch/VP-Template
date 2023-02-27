#pragma once

#include "Networking.h"
#include "Containers/RingBuffer.h"

class FDataPacket;


class FTCPMessaging :
    public FRunnable
{
public:

	FTCPMessaging();
	~FTCPMessaging();

	
	//Try to connect to a socket with the specified endpoint.
	//If RetryAttempts is not 0 it will try to connect if attempts fail until it meets the number or forever if it is max of uint32.
	//Non-blocking, returns immediately as it will run async.
	void ConnectToSocket(
		const FIPv4Endpoint& RemoteEndpoint,
		uint32 RetryAttempts = 0,
		const FTimespan& RetryInterval = { 0, 0, 1 },
		uint32 SendBufferSize = 1024, 
		uint32 ReceiveBufferSize = 0);

	//Add a message to the message queue, non-blocking, spawns async task.
	void Send(const TSharedRef<TArray<uint8>>& Data);
	void Send(const FDataPacket& Packet);

	//Do not call yourself, will be called automatically
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

private:

	struct FTCPMessage
	{
		TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Data;
	};

	void Update();

	TSharedPtr<FSocket> Socket;
	TSharedPtr<FInternetAddr> RemoteAddress;

	TAtomic<uint32> ConnectionTries;
	TAtomic<uint32> MaxConnectionTries;
	TAtomic<FTimespan> ConnectionTryInterval;

	TQueue<FTCPMessage, EQueueMode::Mpsc> MessageQueue;

	bool Running;
	FTimespan WaitTime;

	TUniquePtr<FRunnableThread> Thread;
	FEventRef UpdateEvent;

};