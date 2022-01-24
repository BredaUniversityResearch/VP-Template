#include "BMCCForegroundThreadCallbackHandler.h"

#include "BMCCCommandMeta.h"

void FBMCCForegroundThreadCallbackHandler::OnDataReceived(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView)
{
	m_MessageQueue.Enqueue(CachedMessage(Source, Header, CommandMetaData, ArrayView));
}

void FBMCCForegroundThreadCallbackHandler::DispatchPendingMessages()
{
	while (!m_MessageQueue.IsEmpty())
	{
		CachedMessage message{};
		m_MessageQueue.Dequeue(message);
		message.CommandMetaData->DeserializeAndDispatch(this, message.Source, message.SerializedData);
	}
}
