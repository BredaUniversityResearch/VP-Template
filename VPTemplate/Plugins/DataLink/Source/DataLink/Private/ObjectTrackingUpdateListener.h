#pragma once
#include "TrackerTransformUpdates.h"

class FTCPMessaging;

class FObjectTrackingUpdateListener : public ITrackerTransformUpdateListener
{

public:

    FObjectTrackingUpdateListener(const TSharedRef<FTCPMessaging>& Messaging);
    virtual ~FObjectTrackingUpdateListener() override = default;

    virtual void OnUpdate(const FTrackerTransformUpdate& Update) override;

private:

    TSharedRef<FTCPMessaging> Messaging;

};