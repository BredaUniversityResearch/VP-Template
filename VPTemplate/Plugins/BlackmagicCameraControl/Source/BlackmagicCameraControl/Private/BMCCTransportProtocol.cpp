#include "BMCCTransportProtocol.h"

#include "BMCCCommandHeader.h"
#include "BMCCCommandMeta.h"
#include "BMCCPacketHeader.h"
#include "BlackmagicCameraControl.h"

void BMCCTransportProtocol::DecodeStream(const TArrayView<uint8>& Stream, BMCCDeviceHandle Handle, IBMCCDataReceivedHandler* Dispatcher)
{
	const uint8* data = Stream.GetData();

	int bytesProcessed = 0;
	const FBMCCPacketHeader* packet = reinterpret_cast<const FBMCCPacketHeader*>(Stream.GetData());
	bytesProcessed += sizeof(FBMCCPacketHeader);
	while (Stream.Num() - bytesProcessed >= packet->PacketSize)
	{
		const BMCCCommandHeader* command = reinterpret_cast<const BMCCCommandHeader*>(data + bytesProcessed);
		bytesProcessed += sizeof(BMCCCommandHeader);

		int bytesRemaining = static_cast<int>(Stream.Num()) - bytesProcessed;
		UE_LOG(LogBlackmagicCameraControl, Log, TEXT("Received Message %i.%i. With data %s"), command->Identifier.Category, command->Identifier.Parameter, *BytesToHex(data + bytesProcessed, bytesRemaining));

		const FBMCCCommandMeta* meta = FBMCCCommandMeta::FindMetaForIdentifier(command->Identifier);
		if (meta != nullptr)
		{
			int messageLength = meta->PayloadSize;
			if (messageLength < 0)
			{
				messageLength = FMath::Max(0, bytesRemaining);
			}

			if (Dispatcher != nullptr)
			{
				ensureMsgf(meta->PayloadSize <= packet->PacketSize, TEXT("Metadata mentions payload that is bigger than the actual packet size..."));
				Dispatcher->OnDataReceived(Handle, *command, *meta, TArrayView<uint8_t>(Stream.GetData() + bytesProcessed, messageLength));
			}
			bytesProcessed += meta->PayloadSize;
		}
		else
		{
			UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Failed to find packet meta for ID %i.%i"), command->Identifier.Category, command->Identifier.Parameter);
			break;
		}
	}
}
