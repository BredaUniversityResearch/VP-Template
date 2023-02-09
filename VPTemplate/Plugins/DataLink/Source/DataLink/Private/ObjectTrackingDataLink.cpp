#include "ObjectTrackingDataLink.h"
#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackerSerialId.h"
#include "TCPMessaging.h"
#include "ObjectTrackingPacketData.h"

FObjectTrackingDataLink::FObjectTrackingDataLink(const TSharedPtr<FTCPMessaging>& InMessaging)
    :
Messaging(InMessaging)
{
    if(FPhysicalObjectTracker* objectTrackingModule = FModuleManager::GetModulePtr<FPhysicalObjectTracker>("PhysicalObjectTracker"))
    {
        OnTrackerRegisteredDelegate = objectTrackingModule->OnTrackingComponentRegistered.AddRaw(this, &FObjectTrackingDataLink::OnTrackerRegistered);
    }
}

FObjectTrackingDataLink::~FObjectTrackingDataLink()
{
    OnTrackerRegisteredDelegate.Reset();
}

void FObjectTrackingDataLink::OnTrackerRegistered(UPhysicalObjectTrackingComponent& Component)
{
    const FDelegateHandle trackerUpdateDelegate = Component->OnTrackerTransformUpdate.AddRaw(this, &FObjectTrackingDataLink::OnTrackerTransformUpdate);
    TrackerTransformUpdateDelegates.Add(trackerUpdateDelegate);
}

void FObjectTrackingDataLink::OnTrackerTransformUpdate(
    const UPhysicalObjectTrackingComponent& Component,
    const FTimecode& TimeCode,
    const FTransform& Transform) const
{
    FString trackerAssetName;
    FString trackerSerialId;

    if (Component.TrackerSerialIdAsset != nullptr)
    {
        Component.TrackerSerialIdAsset->GetName(trackerAssetName);
        trackerSerialId = Component.TrackerSerialIdAsset->GetSerialId();
    }

    const TSharedPtr<FObjectTrackingPacketData> packetData = 
        MakeShared<FObjectTrackingPacketData>(trackerAssetName, trackerSerialId, Transform);
    const FDataPacket packet(TimeCode, packetData);

    if(Messaging.IsValid())
    {
        [[maybe_unused]] const bool sent = Messaging->Send(packet);
    }

}