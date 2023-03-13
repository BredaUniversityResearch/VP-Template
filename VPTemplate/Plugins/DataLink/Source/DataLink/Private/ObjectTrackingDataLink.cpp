#include "ObjectTrackingDataLink.h"

#include "ObjectTrackingUpdateListener.h"
#include "TCPMessaging.h"
#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackingComponentRegistry.h"

FObjectTrackingDataLink::FObjectTrackingDataLink(const TSharedRef<FTCPMessaging>& InMessaging)
    :
UpdateListener(MakeShared<FObjectTrackingUpdateListener>(InMessaging))
{
    OnAllModulePhasesCompleteDelegate = FCoreDelegates::OnAllModuleLoadingPhasesComplete.AddLambda(
        [this]()
        {
            if (const FPhysicalObjectTracker* objectTrackingModule = FModuleManager::GetModulePtr<FPhysicalObjectTracker>("PhysicalObjectTracker"))
            {
                const FOnPhysicalObjectTrackingComponentRegistered::FDelegate onRegisteredDelegate =
                    FOnPhysicalObjectTrackingComponentRegistered::FDelegate::CreateRaw(this, &FObjectTrackingDataLink::OnTrackerRegistered);
                const FOnPhysicalObjectTrackingComponentUnregistered::FDelegate onUnregisteredDelegate =
                    FOnPhysicalObjectTrackingComponentUnregistered::FDelegate::CreateRaw(this, &FObjectTrackingDataLink::OnTrackerUnregistered);

                OnTrackerRegisteredDelegate = objectTrackingModule->ObjectTrackingComponents->AddOnComponentRegisteredDelegate(onRegisteredDelegate);
                OnTrackerUnregisteredDelegate = objectTrackingModule->ObjectTrackingComponents->AddOnComponentUnregisteredDelegate(onUnregisteredDelegate);

                //Register listener for all the trackers that have already registered.
                const auto& currentTrackers = objectTrackingModule->ObjectTrackingComponents->GetCurrentObjectTrackingComponents();
                for (auto& tracker : currentTrackers)
                {
                    if (tracker != nullptr)
                    {
                        OnTrackerRegistered(tracker);
                    }
                }
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(23344556, 10.f, FColor::Red, "Couldn't set up ObjectTrackingDataLink as the PhysicalObjectTracker module wasn't found.");
            }

        	OnAllModulePhasesCompleteDelegate.Reset();
        });

}

FObjectTrackingDataLink::~FObjectTrackingDataLink()
{
    OnTrackerRegisteredDelegate.Reset();
    OnTrackerUnregisteredDelegate.Reset();

    if(const FPhysicalObjectTracker* objectTrackingModule = FModuleManager::GetModulePtr<FPhysicalObjectTracker>("PhysicalObjectTracker"))
    {
        //Unregister listener for all the trackers that have been registered,
        //in case of the module destroying earlier than the PhysicalObjectTracker module so it does not try to dereference the shared ptr to this while it is destroyed.
        const auto& currentTrackers = objectTrackingModule->ObjectTrackingComponents->GetCurrentObjectTrackingComponents();
        for(auto& tracker : currentTrackers)
        {
            if(tracker != nullptr)
            {
                OnTrackerUnregistered(tracker);
            }
        }
    }
}

void FObjectTrackingDataLink::OnTrackerRegistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component) const
{check(Component != nullptr)
    Component->TransformUpdates.AddListener(UpdateListener);
}

void FObjectTrackingDataLink::OnTrackerUnregistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component) const
{
    check(Component != nullptr)
    Component->TransformUpdates.RemoveListener(UpdateListener);
}