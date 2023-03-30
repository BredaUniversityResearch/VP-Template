#include "BMCCDispatcher.h"

#include "BMCCCommandMeta.h"
#include "BMCCService.h"

UBMCCDispatcher::CachedMessage::CachedMessage(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView)
	: Source(Source)
	, Header(Header)
	, CommandMetaData(&CommandMetaData)
	, SerializedData(ArrayView)
{
}

void UBMCCDispatcher::PostInitProperties()
{
	UObject::PostInitProperties();
	if (!HasAnyFlags(EObjectFlags::RF_ClassDefaultObject))
	{
		IModularFeatures::Get().GetModularFeature<FBMCCService>(FBMCCService::GetModularFeatureName()).SubscribeMessageReceivedHandler(this);
	}
}

void UBMCCDispatcher::BeginDestroy()
{
	if (!HasAnyFlags(EObjectFlags::RF_ClassDefaultObject))
	{
		IModularFeatures::Get().GetModularFeature<FBMCCService>(FBMCCService::GetModularFeatureName()).UnsubscribeMessageReceivedHandler(this);
	}
	UObject::BeginDestroy();
}

void UBMCCDispatcher::Tick(float DeltaTime)
{
	DispatchPendingMessages();
}
