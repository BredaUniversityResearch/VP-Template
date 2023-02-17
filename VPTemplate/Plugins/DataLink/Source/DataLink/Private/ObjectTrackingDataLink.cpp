#include "ObjectTrackingDataLink.h"
#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackerSerialId.h"
#include "PhysicalObjectTrackingComponentRegistry.h"
#include "TCPMessaging.h"
#include "ObjectTrackingPacketData.h"

FObjectTrackingDataLink::FObjectTrackingDataLink(const TSharedPtr<FTCPMessaging>& InMessaging)
    :
Messaging(InMessaging)
{
    if(FPhysicalObjectTracker* objectTrackingModule = FModuleManager::GetModulePtr<FPhysicalObjectTracker>("PhysicalObjectTracker"))
    {
        //Register listener for all the trackers that have already registered.
        const auto& currentTrackers = objectTrackingModule->ObjectTrackingComponents.GetCurrentObjectTrackingComponents();
        for(auto& tracker : currentTrackers)
        {
            if(tracker != nullptr)
            {
                OnTrackerRegistered(tracker);
            }
        }

        const FOnPhysicalObjectTrackingComponentRegistered::FDelegate onRegisteredDelegate =
            FOnPhysicalObjectTrackingComponentRegistered::FDelegate::CreateRaw(this, &FObjectTrackingDataLink::OnTrackerRegistered);
        const FOnPhysicalObjectTrackingComponentUnregistered::FDelegate onUnregisteredDelegate =
            FOnPhysicalObjectTrackingComponentUnregistered::FDelegate::CreateRaw(this, &FObjectTrackingDataLink::OnTrackerUnregistered);

        OnTrackerRegisteredDelegate = objectTrackingModule->ObjectTrackingComponents.AddOnComponentRegisteredDelegate(onRegisteredDelegate);
        OnTrackerUnregisteredDelegate = objectTrackingModule->ObjectTrackingComponents.AddOnComponentUnregisteredDelegate(onUnregisteredDelegate);
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
        const auto& currentTrackers = objectTrackingModule->ObjectTrackingComponents.GetCurrentObjectTrackingComponents();
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