#pragma once
#include "BMCCDeviceHandle.h"

class IBMCCDataReceivedHandler;

class BMCCTransportProtocol
{
public:
	static void DecodeStream(const TArrayView<uint8>& Stream, BMCCDeviceHandle Handle, IBMCCDataReceivedHandler* Dispatcher);
};
