#pragma once

class UPhysicalObjectTrackingComponent;
class FTCPMessaging;

class FObjectTrackingDataLink
{
public:

    FObjectTrackingDataLink(const TSharedPtr<FTCPMessaging>& Messaging);
    ~FObjectTrackingDataLink();

private:

    void OnTrackerRegistered(UPhysicalObjectTrackingComponent& Component);
    void OnTrackerTransformUpdate(
        const UPhysicalObjectTrackingComponent& Component, 
        const FTimecode& TimeCode, 
        const FTransform& Transform) const;

    FDelegateHandle OnTrackerRegisteredDelegate;
    TArray<FDelegateHandle> TrackerTransformUpdateDelegates;

    TSharedPtr<FTCPMessaging> Messaging;

};