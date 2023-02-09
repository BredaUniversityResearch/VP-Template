#pragma once

class UPhysicalObjectTrackingComponent;
class FTCPMessaging;

class FObjectTrackingDataLink
{
public:

    FObjectTrackingDataLink(const TSharedPtr<FTCPMessaging>& Messaging);
    ~FObjectTrackingDataLink();

private:

    void OnTrackerRegistered(TObjectPtr<UPhysicalObjectTrackingComponent> Component);
    void OnTrackerTransformUpdate(
        TObjectPtr<UPhysicalObjectTrackingComponent> Component, 
        const FTimecode& TimeCode, 
        const FTransform& Transform) const;

    FDelegateHandle OnTrackerRegisteredDelegate;
    TMap<TObjectPtr<UPhysicalObjectTrackingComponent>, FDelegateHandle> TrackerTransformUpdateDelegates;

    TSharedPtr<FTCPMessaging> Messaging;

};