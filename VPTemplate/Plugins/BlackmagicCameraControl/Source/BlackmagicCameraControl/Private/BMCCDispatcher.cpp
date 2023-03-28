#include "BMCCDispatcher.h"

#include "BMCCCommandMeta.h"

UBMCCDispatcher::CachedMessage::CachedMessage(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView)
	: Source(Source)
	, Header(Header)
	, CommandMetaData(&CommandMetaData)
	, SerializedData(ArrayView)
{
}

void UBMCCDispatcher::Tick(float DeltaTime)
{
	DispatchPendingMessages();
}
