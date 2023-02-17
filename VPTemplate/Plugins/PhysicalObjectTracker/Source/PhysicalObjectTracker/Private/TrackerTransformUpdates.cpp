#include "TrackerTransformUpdates.h"

FTrackerTransformUpdates::FTrackerTransformUpdates()
    :
ListenerPipe(TEXT("TrackerTransformUpdatesListenerPipe"))
{}

void FTrackerTransformUpdates::Update(const FTrackerTransformUpdate& Update)
{
    const auto taskName = TEXT("PhysicalObjectTrackerTransformUpdate");
    const auto updateListenerFunction = [this, Update]
    {
        if (Listener.IsValid()) { Listener->OnUpdate(Update); }
    };

    if(previousTask.IsValid() && !previousTask.IsCompleted())
    {
        previousTask = ListenerPipe.Launch(
            taskName,
            updateListenerFunction,
            UE::Tasks::Prerequisites(previousTask));
    }
    else
    {
        previousTask = ListenerPipe.Launch(
            taskName,
            updateListenerFunction);
    }
}

void FTrackerTransformUpdates::AddListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener)
{
    const auto taskName = TEXT("PhysicalObjectTrackerTransformUpdatesAddListener");
    const auto addListenerFunction = [this, InListener]
    {
        Listener = InListener;
    };

    if(previousTask.IsValid() && !previousTask.IsCompleted())
    {
        previousTask = ListenerPipe.Launch(
            taskName,
            addListenerFunction,
            UE::Tasks::Prerequisites(previousTask));
    }
    else
    {
        previousTask = ListenerPipe.Launch(
            taskName,
            addListenerFunction);
    }
}

void FTrackerTransformUpdates::RemoveListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener)
{
    const auto taskName = TEXT("PhysicalObjectTrackerTransformUpdatesRemoveListener");
    const auto removeListenerFunction = [this, InListener]
    {
        if (Listener == InListener)
        {
            Listener.Reset();
        }
    };

    if(previousTask.IsValid() && !previousTask.IsCompleted())
    {
        previousTask = ListenerPipe.Launch(
            taskName,
            removeListenerFunction,
            UE::Tasks::Prerequisites(previousTask));
    }
    else
    {
        previousTask = ListenerPipe.Launch(
            taskName,
            removeListenerFunction);
    }
}

bool FTrackerTransformUpdates::HasListener() const
{
    return Listener.IsValid();
}

FTrackerTransformUpdate::FTrackerTransformUpdate(
    const FTimecode& InTimeCode,
    const FString& InTrackerSerialId,
    const FString& InTrackerSerialIdAssetName,
    const FTransform& InTransform)
        :
TimeCode(InTimeCode),
TrackerSerialId(InTrackerSerialId),
TrackerSerialIdAssetName(InTrackerSerialIdAssetName),
Transformation(InTransform)
{}

ITrackerTransformUpdateListener::~ITrackerTransformUpdateListener() = default;