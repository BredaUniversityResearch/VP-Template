#pragma once
#include "Containers/RingBuffer.h"
#include "Tasks/Task.h"

struct FTrackerTransformUpdate
{
    FTrackerTransformUpdate(
        const FTimecode& InTimeCode,
        const FString& InTrackerSerialId,
        const FString& InTrackerSerialIdAssetName,
        const FTransform& InTransform);

    const FTimecode TimeCode;
    const FString TrackerSerialId;
    const FString TrackerSerialIdAssetName;
    const FTransform Transformation;
};

class PHYSICALOBJECTTRACKER_API ITrackerTransformUpdateListener
{

public:

    virtual ~ITrackerTransformUpdateListener() = 0;
    
    virtual void OnUpdate(const FTrackerTransformUpdate& Update) = 0;

};

class PHYSICALOBJECTTRACKER_API FTrackerTransformUpdates
{

public:

    FTrackerTransformUpdates();
    
    //Launches a new task to update the listener with an update.
    //Non-blocking, returns immediately
    void Update(const FTrackerTransformUpdate& Update);

    void AddListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener);
    void RemoveListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener);

    bool HasListener() const;

private:

    TSharedPtr<ITrackerTransformUpdateListener> Listener;

    UE::Tasks::FPipe ListenerPipe;
    UE::Tasks::FTask previousTask;

};