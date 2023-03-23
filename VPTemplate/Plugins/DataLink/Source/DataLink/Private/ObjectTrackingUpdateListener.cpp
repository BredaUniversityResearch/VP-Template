#include "ObjectTrackingUpdateListener.h"

#include "ObjectTrackingPacketData.h"
#include "TCPMessaging.h"

FObjectTrackingUpdateListener::FObjectTrackingUpdateListener(const TSharedRef<FTCPMessaging>& InMessaging)
    :
Messaging(InMessaging)
{}

void FObjectTrackingUpdateListener::OnUpdate(const FTrackerTransformUpdate& Update)
{
    const TSharedPtr<FObjectTrackingPacketData> packetData =
        MakeShared<FObjectTrackingPacketData>(
            Update.TrackerSerialIdAssetName,
            Update.TrackerSerialId,
            Update.Transformation);
    const FDataPacket packet(Update.TimeCode, packetData);
    Messaging->Send(packet);
}