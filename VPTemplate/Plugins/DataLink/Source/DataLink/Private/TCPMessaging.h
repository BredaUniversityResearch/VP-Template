#pragma once

#include "Networking.h"
#include "Containers/RingBuffer.h"

class FDataPacket;
class FTCPConnection;

class FTCPMessaging :
    public FRunnable
{
public:

	FTCPMessaging(uint32 MaxMessageQueueSize = 0);
	~FTCPMessaging();

	
	//Try to connect to a socket with the specified endpoint.
	//If RetryAttempts is not 0 it will try to connect if attempts fail until it meets the number or forever if it is max of uint32.
	//Non-blocking, returns immediately as it will run async.
	void ConnectSocket(
		const FIPv4Endpoint& RemoteEndpoint,
		const FTimespan& RetryInterval = { 0, 0, 1 },
		uint32 MaxRetryAttempts = 0,
		bool ResetQueue = false,
		uint32 SendBufferSize = 1024, 
		uint32 ReceiveBufferSize = 0);

	void DisconnectSocket();
	void SetMaxMessageQueueSize(uint32 MaxMessageQueueSize);

	bool GetConnectionState(FIPv4Endpoint& ConnectionEndpoint) const;
	uint32 GetMaxMessageQueueSize() const;

	//Add a message to the message queue, non-blocking, spawns async task.
	bool Send(const TSharedRef<TArray<uint8>>& Data);
	bool Send(const FDataPacket& Packet);

	//Do not call yourself, will be called automatically
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override {}

private:

	using FTCPMessage = TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe>; //Has to be SharedPtr instead of SharedRef due to use in TQueue

	void Update();
	void PopMessageFromQueue(bool IsDiscard = false);

	TSharedPtr<FTCPConnection> Connection;	//Uses SharedPtr to get the thread-safe access
	TQueue<FTCPMessage, EQueueMode::Mpsc> MessageQueue;
	TAtomic<uint32> MessageQueueSize;
	TAtomic<uint32> MaxMessageQueueSize;
	uint64 DiscardedMessageCount;

	bool Running;
	FTimespan WaitTime;

	FEventRef UpdateEvent;
	TUniquePtr<FRunnableThread> Thread;

};