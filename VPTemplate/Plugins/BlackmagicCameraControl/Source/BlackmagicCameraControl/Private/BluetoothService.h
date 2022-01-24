#pragma once

#include "BMCCDeviceHandle.h"

struct FBMCCCommandPayloadBase;
struct FBMCCCommandIdentifier;
class IBMCCDataReceivedHandler;
struct BMCCCommandHeader;

class FBluetoothService
{
	class FBluetoothWorker;
	friend class FBluetoothWorkerRunnable;
public:
	FBluetoothService(IBMCCDataReceivedHandler* DataReceivedHandler);
	~FBluetoothService();
	
	void QueryManufacturer(BMCCDeviceHandle Target);
	void QueryCameraModel(BMCCDeviceHandle Target);

	void SendToCamera(BMCCDeviceHandle Target, const FBMCCCommandIdentifier& CommandId, const FBMCCCommandPayloadBase& Command);

private:
	TUniquePtr<FBluetoothWorker> Worker;
};

