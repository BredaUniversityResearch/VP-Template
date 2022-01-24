#pragma once
#include "BMCCCallbackHandler.h"
#include "BMCCCommandHeader.h"

//Callback handler specialisation for caching messags and pulling them to the main game thread before dispatching it to the associated handling function.
class FBMCCForegroundThreadCallbackHandler
	: public IBMCCCallbackHandler
{
	struct CachedMessage
	{
		CachedMessage() = default;;
		CachedMessage(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView);

		BMCCDeviceHandle Source{};
		BMCCCommandHeader Header{};
		const FBMCCCommandMeta* CommandMetaData{};
		TArray<uint8> SerializedData{};
	};
public:
	virtual void OnDataReceived(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView) override final;
	void DispatchPendingMessages();

private:
	TQueue<CachedMessage> m_MessageQueue;
};