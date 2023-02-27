#pragma once
#include "TrackerTransformUpdates.h"

class UPhysicalObjectTrackingComponent;
class FTCPMessaging;

class FObjectTrackingDataLink : public ITrackerTransformUpdateListener
{
public:

    FObjectTrackingDataLink(const TSharedPtr<FTCPMessaging>& Messaging);
    virtual ~FObjectTrackingDataLink() override;

private:

    void OnTrackerRegistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component);
    void OnTrackerUnregistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component);

    virtual void OnUpdate(const FTrackerTransformUpdate& Update) override;;

    FDelegateHandle OnTrackerRegisteredDelegate;
    FDelegateHandle OnTrackerUnregisteredDelegate;

    TSharedPtr<FTCPMessaging> Messaging;

};