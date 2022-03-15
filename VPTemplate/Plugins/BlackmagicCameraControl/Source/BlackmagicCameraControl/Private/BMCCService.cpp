#include "BMCCService.h"

#include "BlackmagicCameraControl.h"
#include "BluetoothService.h"
#include "BMCCCallbackHandler.h"
#include "BMCCDispatcher.h"
#include "BMCCBattery_Info.h"
#include "BMCCCommandHeader.h"
#include "BMCCCommandMeta.h"

class FBMCCService::Pimpl
{
public:
	TUniquePtr<FBluetoothService> m_BluetoothService;

	TArray<IBMCCCallbackHandler*> CallbackHandlers;
};

FBMCCService::FBMCCService()
	: m_Data(MakeUnique<Pimpl>())
	, DefaultDispatcher(NewObject<UBMCCDispatcher>())
{
	DefaultDispatcher->AddToRoot();
	m_Data->m_BluetoothService = MakeUnique<FBluetoothService>(this);
	SubscribeMessageReceivedHandler(DefaultDispatcher);
}

FBMCCService::~FBMCCService()
{
	UnsubscribeMessageReceivedHandler(DefaultDispatcher);
	if (IsValid(DefaultDispatcher))
	{
		DefaultDispatcher->RemoveFromRoot();
	}
}

void FBMCCService::Tick(float DeltaTime)
{
}

void FBMCCService::OnDataReceived(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8_t>& SerializedData)
{
	for (const auto it : m_Data->CallbackHandlers)
	{
		it->OnDataReceived(Source, Header, CommandMetaData, SerializedData);
	}
}

void FBMCCService::BroadcastCommand(const FBMCCCommandIdentifier& Identifier, const FBMCCCommandPayloadBase& Command) const
{
	m_Data->m_BluetoothService->SendToCamera(BMCCDeviceHandle_Broadcast, Identifier, Command);
}

void FBMCCService::SubscribeMessageReceivedHandler(IBMCCCallbackHandler* a_CallbackHandler)
{
	m_Data->CallbackHandlers.Emplace(a_CallbackHandler);
}

void FBMCCService::UnsubscribeMessageReceivedHandler(IBMCCCallbackHandler* a_CallbackHandler)
{
	m_Data->CallbackHandlers.RemoveSingleSwap(a_CallbackHandler);
}

UBMCCDispatcher* FBMCCService::GetDefaultDispatcher() const
{
	return DefaultDispatcher;
}
