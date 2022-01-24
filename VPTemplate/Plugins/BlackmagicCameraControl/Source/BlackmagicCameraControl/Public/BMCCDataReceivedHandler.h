#pragma once

#include "BMCCDeviceHandle.h"

class FBMCCCommandMeta;
struct BMCCCommandHeader;

class IBMCCDataReceivedHandler
{
public:
	virtual ~IBMCCDataReceivedHandler() = default;
	virtual void OnDataReceived(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8_t>& SerializedData) = 0;
};
