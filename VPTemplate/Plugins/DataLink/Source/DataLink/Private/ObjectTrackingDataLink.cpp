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

void FObjectTrackingDataLink::OnTrackerRegistered(TSharedRef<UPhysicalObjectTrackingComponent> Component)
{
    FDelegateHandle trackerUpdateDelegate =  Component->OnTrackerTransformUpdate.AddRaw(this, &FObjectTrackingDataLink::OnTrackerTransformUpdate);
    //TODO: is it necessary to keep track of the trackers that have been registered?
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
    FDataPacket packet(TimeCode, packetData);

}