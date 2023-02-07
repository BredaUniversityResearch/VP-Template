#pragma once

class UPhysicalObjectTrackingComponent;
class FTCPMessaging;

class FObjectTrackingDataLink
{
public:

    FObjectTrackingDataLink(const TSharedPtr<FTCPMessaging>& Messaging);

private:

    void OnTrackerRegistered(TSharedRef<UPhysicalObjectTrackingComponent> Component);
    void OnTrackerTransformUpdate(
        const UPhysicalObjectTrackingComponent& Component, 
        const FTimecode& TimeCode, 
        const FTransform& Transform) const;

    FDelegateHandle OnTrackerRegisteredDelegate;
    TSharedPtr<FTCPMessaging> Messaging;

};