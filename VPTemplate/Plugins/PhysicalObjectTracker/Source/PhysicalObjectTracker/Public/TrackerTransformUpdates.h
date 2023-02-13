#pragma once
#include "Containers/RingBuffer.h"
#include "Tasks/Task.h"

class PHYSICALOBJECTTRACKER_API UPhysicalObjectTrackingComponent;

struct FTransformUpdate
{
    FTransformUpdate(
        const FTimecode& InTimeCode,
        const FTransform& InTransform);

    const FTimecode TimeCode;
    const FTransform Transformation;
};

class PHYSICALOBJECTTRACKER_API ITrackerTransformUpdateListener
{

public:

    virtual ~ITrackerTransformUpdateListener() = 0;

    //TODO: it maybe necessary to make the Component parameter thread-safe but not blocking in some way?
    virtual void OnUpdate(TObjectPtr<UPhysicalObjectTrackingComponent> Component, const FTransformUpdate& Update) = 0;

};

class PHYSICALOBJECTTRACKER_API FTrackerTransformUpdates
{

public:
    
    //Launches a new task to update the listener with an update.
    //Non-blocking, returns immediately
    void Update(
        TObjectPtr<UPhysicalObjectTrackingComponent> Component,
        const FTransformUpdate& Update);

    void AddListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener);
    void RemoveListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener);

    bool HasListener() const;

private:

    //Can be updated to a vector if multiple listeners should be possible.
    TSharedPtr<ITrackerTransformUpdateListener> Listener;
    UE::Tasks::FTask previousTask;

};