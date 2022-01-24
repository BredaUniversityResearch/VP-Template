#include "BluetoothService.h"

#include "WinRT.h"
#include "BlackmagicCameraControl.h"

#include "BlackMagicBluetoothGUID.h"
#include "BluetoothDeviceConnection.h"
#include "BMCCCommandHeader.h"
#include "BMCCCommandMeta.h"
#include "BMCCPacketHeader.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace GenericAttributeProfile;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Storage::Streams;

namespace
{
	void CreateCommandPackage(const FBMCCCommandMeta& Meta, const FBMCCCommandPayloadBase& Payload, Buffer& TargetBuffer)
	{
		FBMCCPacketHeader header;
		int payloadSize = static_cast<int>((sizeof(FBMCCPacketHeader) + sizeof(BMCCCommandHeader) + Meta.PayloadSize));
		int padBytes = ((payloadSize + 3) & ~3) - payloadSize;
		assert(payloadSize + padBytes < 0xFF);
		header.PacketSize = static_cast<uint8_t>(payloadSize - sizeof(FBMCCPacketHeader));

		BMCCCommandHeader commandHeader(Meta.CommandIdentifier);

		uint8_t* data = TargetBuffer.data();
		std::memcpy(data, &header, sizeof(FBMCCPacketHeader));
		std::memcpy(data + sizeof(FBMCCPacketHeader), &commandHeader, sizeof(commandHeader));
		std::memcpy(data + sizeof(FBMCCPacketHeader) + sizeof(BMCCCommandHeader), &Payload, Meta.PayloadSize);

		TargetBuffer.Length(payloadSize + padBytes);
	}
}

struct FDataQueueEntry
{
	FDataQueueEntry() = default;
	FDataQueueEntry(BMCCDeviceHandle TargetDevice, Buffer&& DataToSend)
		: Target(TargetDevice)
		, DataToSend(MoveTemp(DataToSend))
	{
	}

	BMCCDeviceHandle Target{ 0 };
	Buffer DataToSend{ nullptr };
};

class FBluetoothWorkerRunnable : public FRunnable
{
public:
	FBluetoothWorkerRunnable(FBluetoothService::FBluetoothWorker* Worker);

	virtual uint32 Run() override;

	FBluetoothService::FBluetoothWorker* OwningWorker;
	bool QuitRequested{ false };
};

class FBluetoothService::FBluetoothWorker
{
public:
	FBluetoothWorker(IBMCCDataReceivedHandler* TargetDataReceivedHandler);
	~FBluetoothWorker();

	void OnDeviceAdded(const DeviceWatcher& Watcher, const DeviceInformation& DeviceInfo);
	void OnDeviceRemoved(const DeviceWatcher& Watcher, const DeviceInformationUpdate& DeviceInfo);
	void OnDeviceUpdated(const DeviceWatcher& Watcher, const DeviceInformationUpdate& DeviceInfo);

	void TryConnectToDevice(const winrt::hstring& DeviceId);
	FBluetoothDeviceConnection* FindDeviceByHandle(BMCCDeviceHandle Target);
	void EnqueueSendPackage(BMCCDeviceHandle Target, Buffer&& DataToSend);

	static void BackgroundService(FBluetoothWorker& Target);

	IBMCCDataReceivedHandler* TargetDataReceivedHandler;
	TArray<TUniquePtr<FBluetoothDeviceConnection>> ActiveConnections{};

	BMCCDeviceHandle m_LastUsedHandle{ 0 };
	DeviceWatcher BLEDeviceWatcher;
	winrt::event_token BLEDeviceWatcher_Added;
	winrt::event_token BLEDeviceWatcher_Removed;
	winrt::event_token BLEDeviceWatcher_Updated;

	FCriticalSection ReconnectQueueLock;
	TArray<winrt::hstring> DeviceIdConnectRetryQueue;

	FBluetoothWorkerRunnable UpdateThreadRunnable;
	FRunnableThread* UpdateThread;

	TQueue<FDataQueueEntry> ReceiveQueue;
};

FBluetoothWorkerRunnable::FBluetoothWorkerRunnable(FBluetoothService::FBluetoothWorker* Worker)
	: OwningWorker(Worker)
{
}

uint32 FBluetoothWorkerRunnable::Run()
{
	while (!QuitRequested)
	{
		if (OwningWorker->DeviceIdConnectRetryQueue.Num() > 0)
		{
			FScopeLock lock(&OwningWorker->ReconnectQueueLock);
			winrt::hstring result = MoveTemp(OwningWorker->DeviceIdConnectRetryQueue[0]);
			OwningWorker->DeviceIdConnectRetryQueue.RemoveAt(0);
			OwningWorker->TryConnectToDevice(result);
		}
	}
	return 0;
}

FBluetoothService::FBluetoothWorker::FBluetoothWorker(IBMCCDataReceivedHandler* TargetDataReceivedHandler)
	: TargetDataReceivedHandler(TargetDataReceivedHandler)
	, BLEDeviceWatcher(DeviceInformation::CreateWatcher(
		BluetoothLEDevice::GetDeviceSelectorFromPairingState(true),
		{}, DeviceInformationKind::Device))
	, UpdateThreadRunnable(this)
{
	BLEDeviceWatcher_Added = BLEDeviceWatcher.Added({this, &FBluetoothWorker::OnDeviceAdded });
	BLEDeviceWatcher_Removed = BLEDeviceWatcher.Removed({ this, &FBluetoothWorker::OnDeviceRemoved });
	BLEDeviceWatcher_Updated = BLEDeviceWatcher.Updated({ this, &FBluetoothWorker::OnDeviceUpdated });
	BLEDeviceWatcher.Start();

	UpdateThread = FRunnableThread::Create(&UpdateThreadRunnable, TEXT("FBluetoothWorker::BackgroundService"));

	//ActiveConnections.Emplace(MakeUnique<FBluetoothDeviceConnection>(++m_LastUsedHandle, TargetDataReceivedHandler, FBluetoothDeviceConnection::ELoopbackDevice::Loopback));
}

FBluetoothService::FBluetoothWorker::~FBluetoothWorker()
{
	BLEDeviceWatcher.Stop();
	BLEDeviceWatcher.Added(BLEDeviceWatcher_Added);
	BLEDeviceWatcher.Removed(BLEDeviceWatcher_Removed);
	BLEDeviceWatcher.Updated(BLEDeviceWatcher_Updated);

	UpdateThreadRunnable.QuitRequested = true;
	UpdateThread->WaitForCompletion();
}

void FBluetoothService::FBluetoothWorker::OnDeviceAdded(const DeviceWatcher&, const DeviceInformation& DeviceInfo)
{
	UE_LOG(LogBlackmagicCameraControl, Display, TEXT("OnDeviceAdded: %s"), DeviceInfo.Id().c_str());
	UE_LOG(LogBlackmagicCameraControl, Display, TEXT("\tName: %s"), DeviceInfo.Name().c_str());

	TryConnectToDevice(DeviceInfo.Id());
}

void FBluetoothService::FBluetoothWorker::OnDeviceRemoved(const DeviceWatcher&, const DeviceInformationUpdate& DeviceInfo)
{
	UE_LOG(LogBlackmagicCameraControl, Display, TEXT("Device disconnected : %s"), DeviceInfo.Id().c_str());
	for (const auto& connectedDevice : ActiveConnections)
	{
		if (connectedDevice->m_Device.DeviceId() == DeviceInfo.Id())
		{
			UE_LOG(LogBlackmagicCameraControl, Display, TEXT("\tRemoved active device"), DeviceInfo.Id().c_str());
		}
	}
}

void FBluetoothService::FBluetoothWorker::OnDeviceUpdated(const DeviceWatcher&,	const DeviceInformationUpdate& DeviceInfo)
{
	UE_LOG(LogBlackmagicCameraControl, Verbose, TEXT("\tRemoved active device"), DeviceInfo.Id().c_str());
}

void FBluetoothService::FBluetoothWorker::TryConnectToDevice(const winrt::hstring& DeviceId)
{
	BluetoothLEDevice::FromIdAsync(DeviceId).Completed([this](IAsyncOperation<BluetoothLEDevice> connectedDeviceOp, AsyncStatus)
		{
			const auto& connectedDevice = connectedDeviceOp.GetResults();
			GattDeviceServicesResult servicesResult = connectedDevice.GetGattServicesAsync().get();
			int foundRequiredServices = 0;

			GattDeviceService blackMagicService(nullptr);
			GattDeviceService deviceInformationService(nullptr);

			for (const auto& service : servicesResult.Services())
			{
				if (service.Uuid() == BMBTGUID::BlackMagicServiceUUID)
				{
					blackMagicService = service;
					++foundRequiredServices;
				}
				else if (service.Uuid() == BMBTGUID::DeviceInformationServiceUUID)
				{
					deviceInformationService = service;
					++foundRequiredServices;
				}
			}
			if (foundRequiredServices == 2)
			{
				TUniquePtr<FBluetoothDeviceConnection> deviceConnection = MakeUnique<FBluetoothDeviceConnection>(++m_LastUsedHandle, TargetDataReceivedHandler, connectedDeviceOp.GetResults(), deviceInformationService, blackMagicService);
				if (deviceConnection->IsValid())
				{
					UE_LOG(LogBlackmagicCameraControl, Display, TEXT("Created new active connection for device %s"), connectedDevice.DeviceId().c_str());
					ActiveConnections.Emplace(MoveTemp(deviceConnection));
				}
				else
				{
					UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Tried to create new connection for device, but was not valid. %s"), connectedDevice.DeviceId().c_str());

					{
						FScopeLock lock(&ReconnectQueueLock);
						DeviceIdConnectRetryQueue.Emplace(connectedDevice.DeviceId());
					}
				}
			}
		});
}

FBluetoothDeviceConnection* FBluetoothService::FBluetoothWorker::FindDeviceByHandle(BMCCDeviceHandle Target)
{
	for (const auto& it : ActiveConnections)
	{
		if (it->m_DeviceHandle == Target)
		{
			return it.Get();
		}
	}
	return nullptr;
}

void FBluetoothService::FBluetoothWorker::EnqueueSendPackage(BMCCDeviceHandle Target, Buffer&& DataToSend)
{
	if (Target == BMCCDeviceHandle_Broadcast)
	{
		for (TUniquePtr<FBluetoothDeviceConnection>& connection : ActiveConnections)
		{
			connection->SendOutgoingCameraControl(DataToSend);
		}
	}
	else
	{
		checkNoEntry();// Not yet implemented
	}
}

FBluetoothService::FBluetoothService(IBMCCDataReceivedHandler* DataReceivedHandler)
{
	Worker = MakeUnique<FBluetoothWorker>(DataReceivedHandler);
}

FBluetoothService::~FBluetoothService() = default;

void FBluetoothService::QueryManufacturer(BMCCDeviceHandle Target)
{
	FBluetoothDeviceConnection* connection = Worker->FindDeviceByHandle(Target);
	if (connection != nullptr)
	{
		connection->QueryCameraManufacturer();
	}
	else
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Failed to get camera manufacturer, device handle not found"));
	}
}

void FBluetoothService::QueryCameraModel(BMCCDeviceHandle Target)
{
	FBluetoothDeviceConnection* connection = Worker->FindDeviceByHandle(Target);
	if (connection != nullptr)
	{
		connection->QueryCameraModel();
	}
	else
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Failed to get camera model, device handle not found"));
	}
}

void FBluetoothService::SendToCamera(BMCCDeviceHandle Target, const FBMCCCommandIdentifier& CommandId, const FBMCCCommandPayloadBase& Command)
{
	const FBMCCCommandMeta* meta = FBMCCCommandMeta::FindMetaForIdentifier(CommandId);
	if (meta == nullptr)
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Failed to find meta for command identifier %i.%i. Message was not sent"), CommandId.Category, CommandId.Parameter);
		return;
	}
	Buffer serializedDataBuffer(128);
	CreateCommandPackage(*meta, Command, serializedDataBuffer);
	Worker->EnqueueSendPackage(Target, MoveTemp(serializedDataBuffer));
}
