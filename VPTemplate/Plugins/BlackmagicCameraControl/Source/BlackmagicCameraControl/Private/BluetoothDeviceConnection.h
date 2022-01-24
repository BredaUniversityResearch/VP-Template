#pragma once
#include "WinRT.h"
#include "BluetoothService.h"

class FBMCCCommandMeta;
class IBMCCDataReceivedHandler;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace GenericAttributeProfile;

class FBluetoothDeviceConnection
{
public:
	enum class ELoopbackDevice
	{
		Loopback
	};

	FBluetoothDeviceConnection(BMCCDeviceHandle DeviceHandle, IBMCCDataReceivedHandler* CallbackService, const BluetoothLEDevice& Device, 
	                           const GattDeviceService& DeviceInformationService, const GattDeviceService& BlackMagicService);
	FBluetoothDeviceConnection(BMCCDeviceHandle DeviceHandle, IBMCCDataReceivedHandler* CallbackService, ELoopbackDevice);

	bool IsValid() const;

	void QueryCameraManufacturer();
	void QueryCameraModel();

	void SendOutgoingCameraControl(const winrt::Windows::Storage::Streams::IBuffer& SerializedPackage) const;

	void OnQueryCameraManufacturerCompleted(const GattReadResult& result);
	void OnReceivedIncomingCameraControl(const winrt::Windows::Storage::Streams::IBuffer& InputData) const;

	void SetupBlackMagicServiceCharacteristics();

	const BMCCDeviceHandle m_DeviceHandle;
	BluetoothLEDevice m_Device;
	GattDeviceService m_DeviceInformationService;
	GattCharacteristic m_DeviceInformation_CameraManufacturer;
	GattCharacteristic m_DeviceInformation_CameraModel;
	GattDeviceService m_BlackMagicService;
	GattCharacteristic m_BlackMagicService_OutgoingCameraControl;
	GattCharacteristic m_BlackMagicService_IncomingCameraControl;

	IBMCCDataReceivedHandler* m_DataReceivedHandler;
};
