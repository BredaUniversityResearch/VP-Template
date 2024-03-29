#include "BluetoothDeviceConnection.h"

#include "AsyncWrapper.h"
#include "BlackMagicBluetoothGUID.h"
#include "BMCCCommandHeader.h"
#include "BMCCCommandMeta.h"
#include "BMCCPacketHeader.h"
#include "BMCCTransportProtocol.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage::Streams;

FBluetoothDeviceConnection::FBluetoothDeviceConnection(BMCCDeviceHandle DeviceHandle, IBMCCDataReceivedHandler* DataReceivedHandler, const BluetoothLEDevice& Device, const GattDeviceService& DeviceInformationService, const GattDeviceService& BlackMagicService)
	: m_DeviceHandle(DeviceHandle)
	, m_Device(Device)
	, m_DeviceInformationService(DeviceInformationService)
	, m_DeviceInformation_CameraManufacturer(DeviceInformationService.GetCharacteristics(BMBTGUID::DeviceInformationService_CameraManufacturer).GetAt(0))
	, m_DeviceInformation_CameraModel(DeviceInformationService.GetCharacteristics(BMBTGUID::DeviceInformationService_CameraModel).GetAt(0))
	, m_BlackMagicService(BlackMagicService)
	, m_BlackMagicService_OutgoingCameraControl(nullptr)
	, m_BlackMagicService_IncomingCameraControl(nullptr)
	, m_DataReceivedHandler(DataReceivedHandler)
{
	SetupBlackMagicServiceCharacteristics();
}

FBluetoothDeviceConnection::FBluetoothDeviceConnection(BMCCDeviceHandle DeviceHandle, IBMCCDataReceivedHandler* CallbackService, ELoopbackDevice)
	: m_DeviceHandle(DeviceHandle)
	, m_Device(nullptr)
	, m_DeviceInformationService(nullptr)
	, m_DeviceInformation_CameraManufacturer(nullptr)
	, m_DeviceInformation_CameraModel(nullptr)
	, m_BlackMagicService(nullptr)
	, m_BlackMagicService_OutgoingCameraControl(nullptr)
	, m_BlackMagicService_IncomingCameraControl(nullptr)
	, m_DataReceivedHandler(CallbackService)
{
	UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Creating Blackmagic Camera Control loopback device with handle %i"), m_DeviceHandle);
}

bool FBluetoothDeviceConnection::IsValid() const
{
	return m_BlackMagicService_IncomingCameraControl != nullptr && m_BlackMagicService_OutgoingCameraControl != nullptr;
}

void FBluetoothDeviceConnection::QueryCameraManufacturer()
{
	BluetoothConnectionStatus status = m_Device.ConnectionStatus();
	IAsyncOperation<GattReadResult> result = m_DeviceInformation_CameraManufacturer.ReadValueAsync(
		BluetoothCacheMode::Uncached);
	result.Completed(AsyncWrapper(this, &FBluetoothDeviceConnection::OnQueryCameraManufacturerCompleted));
}

void FBluetoothDeviceConnection::QueryCameraModel()
{
	IAsyncOperation<GattReadResult> result = m_DeviceInformation_CameraModel.ReadValueAsync(
		BluetoothCacheMode::Uncached);
	result.Completed(AsyncWrapper(this, &FBluetoothDeviceConnection::OnQueryCameraManufacturerCompleted));
}

void FBluetoothDeviceConnection::SendOutgoingCameraControl(const IBuffer& SerializedPackage) const
{
	if (m_Device != nullptr)
	{
		if (m_Device.ConnectionStatus() != BluetoothConnectionStatus::Connected)
			return;

		int packetLength = SerializedPackage.Length();
		IAsyncOperation<GattCommunicationStatus> asyncOp = m_BlackMagicService_OutgoingCameraControl.WriteValueAsync(SerializedPackage, GattWriteOption::WriteWithResponse);
		asyncOp.Completed([packetLength](IAsyncOperation<GattCommunicationStatus> result, AsyncStatus status) {
			if (result.GetResults() == GattCommunicationStatus::Success)
			{
				UE_LOG(LogBlackmagicCameraControl, Verbose, TEXT("Successfully wrote data %i bytes to OutgoingCameraControl"), packetLength);
			}
			else
			{
				UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Failed to writ data to OutgoingCameraControl %s"), winrt::to_hstring(static_cast<int>(result.GetResults())).c_str());
			}
		});
	}
	else
	{
		OnReceivedIncomingCameraControl(SerializedPackage);
	}
}

void FBluetoothDeviceConnection::OnQueryCameraManufacturerCompleted(const GattReadResult& result)
{
	UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("%s"), ANSI_TO_TCHAR(reinterpret_cast<const char*>(result.Value().data())));
}

void FBluetoothDeviceConnection::OnReceivedIncomingCameraControl(const IBuffer& InputData) const
{
	BMCCTransportProtocol::DecodeStream(TArrayView<uint8>(InputData.data(), InputData.Length()), m_DeviceHandle, m_DataReceivedHandler);
}

void FBluetoothDeviceConnection::SetupBlackMagicServiceCharacteristics()
{
	m_BlackMagicService.GetCharacteristicsAsync().Completed([this](const IAsyncOperation<GattCharacteristicsResult>& ResultOp, AsyncStatus) {
		GattCharacteristicsResult result = ResultOp.GetResults();
		if (result.Status() == GattCommunicationStatus::Success)
		{
			for (const GattCharacteristic& characteristic : result.Characteristics())
			{
				if (characteristic.Uuid() == BMBTGUID::BlackMagicService_OutgoingCameraControl)
				{
					m_BlackMagicService_OutgoingCameraControl = characteristic;
				}
				else if (characteristic.Uuid() == BMBTGUID::BlackMagicService_IncomingCameraControl)
				{
					m_BlackMagicService_IncomingCameraControl = characteristic;
				}
			}
			if (m_BlackMagicService_IncomingCameraControl != nullptr && m_BlackMagicService_OutgoingCameraControl != nullptr)
			{
				GattClientCharacteristicConfigurationDescriptorValue newValue = GattClientCharacteristicConfigurationDescriptorValue::Indicate;
				m_BlackMagicService_IncomingCameraControl.WriteClientCharacteristicConfigurationDescriptorAsync(newValue).Completed([this](IAsyncOperation<GattCommunicationStatus>, AsyncStatus)
					{
						m_BlackMagicService_IncomingCameraControl.ValueChanged([this](const GattCharacteristic&, const GattValueChangedEventArgs& Args) {
							OnReceivedIncomingCameraControl(Args.CharacteristicValue());
							});
					});
			}
			else
			{
				UE_LOG(LogBlackmagicCameraControl, Error, TEXT("One or mor BlackMagic service characteristics was not found"));
			}
		}
		else
		{
			UE_LOG(LogBlackmagicCameraControl, Error, TEXT("One or more BlackMagic service characteristics was not found. Communication did not return SUCCESS"));
		}
		});
}
