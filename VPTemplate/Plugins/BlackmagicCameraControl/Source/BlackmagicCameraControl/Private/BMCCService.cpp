#include "BMCCService.h"

#include "Bluetooth/BluetoothService.h"
#include "BMCCCallbackHandler.h"
#include "BMCCDispatcher.h"
#include "BMCCBattery_Info.h"
#include "BMCCCommandHeader.h"
#include "BMCCCommandMeta.h"
#include "EthernetRelay/CameraControlNetworkReceiver.h"

class FBMCCService::Pimpl
{
public:
	TUniquePtr<FBluetoothService> m_BluetoothService;
	TUniquePtr<FCameraControlNetworkReceiver> m_NetworkService;

	TArray<IBMCCCallbackHandler*> CallbackHandlers;
};

FBMCCService::FBMCCService()
	: m_Data(MakeUnique<Pimpl>())
	, DefaultDispatcher(NewObject<UBMCCDispatcher>())
{
	DefaultDispatcher->AddToRoot();
	//m_Data->m_BluetoothService = MakeUnique<FBluetoothService>(this);

	SubscribeMessageReceivedHandler(DefaultDispatcher);
}

FBMCCService::~FBMCCService()
{
	UnsubscribeMessageReceivedHandler(DefaultDispatcher);
	if (IsValid(DefaultDispatcher))
	{
		DefaultDispatcher->RemoveFromRoot();
	}

	if (m_Data->m_NetworkService != nullptr)
	{
		m_Data->m_NetworkService->Stop();
	}
}

void FBMCCService::Tick(float DeltaTime)
{
	if (m_FirstTick)
	{
		m_Data->m_NetworkService = MakeUnique<FCameraControlNetworkReceiver>(this);
		m_Data->m_NetworkService->Start();
		m_FirstTick = false;
	}

	m_Data->m_NetworkService->Update();
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
