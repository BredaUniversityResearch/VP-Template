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
        OnTrackerUnregisteredDelegate = objectTrackingModule->OnTrackingComponentUnregistered.AddRaw(this, &FObjectTrackingDataLink::OnTrackerUnregistered);

        //Register listener for all the trackers that have already registered.
        const auto& currentTrackers = objectTrackingModule->GetCurrentObjectTrackers();
        for(auto& tracker : currentTrackers)
        {
            if(tracker != nullptr)
            {
                OnTrackerRegistered(tracker);
            }
        }
    }
}

FObjectTrackingDataLink::~FObjectTrackingDataLink()
{
    OnTrackerRegisteredDelegate.Reset();
    OnTrackerUnregisteredDelegate.Reset();

    if(const FPhysicalObjectTracker* objectTrackingModule = FModuleManager::GetModulePtr<FPhysicalObjectTracker>("PhysicalObjectTracker"))
    {
        //Unregister listener for all the trackers that have been registered,
        //in case of the module destroying earlier than the PhysicalObjectTracker module so it does not try to dereference the shared ptr to this while it is destroyed.
        const auto& currentTrackers = objectTrackingModule->GetCurrentObjectTrackers();
        for(auto& tracker : currentTrackers)
        {
            if(tracker != nullptr)
            {
                OnTrackerUnregistered(tracker);
            }
        }
    }
}

void FObjectTrackingDataLink::OnTrackerRegistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component)
{
    check(Component != nullptr)
    Component->TransformUpdates.AddListener(TSharedPtr<ITrackerTransformUpdateListener>(this));
}

void FObjectTrackingDataLink::OnTrackerUnregistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component)
{
    check(Component != nullptr)
    Component->TransformUpdates.RemoveListener(TSharedPtr<ITrackerTransformUpdateListener>(this));
}

void FObjectTrackingDataLink::OnUpdate(TObjectPtr<UPhysicalObjectTrackingComponent> Component, const FTransformUpdate& Update)
{
    check(Component != nullptr)
    FString trackerAssetName;
    FString trackerSerialId;

    if (Component->TrackerSerialIdAsset != nullptr)
    {
        Component->TrackerSerialIdAsset->GetName(trackerAssetName);
        trackerSerialId = Component->TrackerSerialIdAsset->GetSerialId();
    }

    const TSharedPtr<FObjectTrackingPacketData> packetData =
        MakeShared<FObjectTrackingPacketData>(trackerAssetName, trackerSerialId, Update.Transformation);
    const FDataPacket packet(Update.TimeCode, packetData);

    if (Messaging.IsValid())
    {
        [[maybe_unused]] const bool sent = Messaging->Send(packet);
    }
}