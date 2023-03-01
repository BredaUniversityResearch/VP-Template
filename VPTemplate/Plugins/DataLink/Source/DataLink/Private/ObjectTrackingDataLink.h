#pragma once

class FObjectTrackingUpdateListener;
class UPhysicalObjectTrackingComponent;
class FTCPMessaging;

class FObjectTrackingDataLink
{
public:
    
    FObjectTrackingDataLink(const TSharedRef<FTCPMessaging>& Messaging);
    ~FObjectTrackingDataLink();

private:

    void OnTrackerRegistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component) const;
    void OnTrackerUnregistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component) const;

    FDelegateHandle OnTrackerRegisteredDelegate;
    FDelegateHandle OnTrackerUnregisteredDelegate;

    TSharedRef<FObjectTrackingUpdateListener> UpdateListener;

};