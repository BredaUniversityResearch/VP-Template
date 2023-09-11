#include "TrackerTransformUpdates.h"

FTrackerTransformUpdates::FTrackerTransformUpdates()
{}

void FTrackerTransformUpdates::Update(const FTrackerTransformUpdate& Update)
{
    const auto taskName = TEXT("PhysicalObjectTrackerTransformUpdate");
    auto updateListenerFunction = [this, Update]
    {
        if (Listener.IsValid()) { Listener->OnUpdate(Update); }
    };

    if(previousTask.IsValid() && !previousTask.IsCompleted())
    {
        previousTask = UE::Tasks::Launch(taskName, MoveTemp(updateListenerFunction), UE::Tasks::Prerequisites(previousTask));

    }
    else
    {
        previousTask = UE::Tasks::Launch(taskName, MoveTemp(updateListenerFunction));
    }
}

void FTrackerTransformUpdates::AddListener(const TSharedRef<ITrackerTransformUpdateListener>& InListener)
{
    const auto taskName = TEXT("PhysicalObjectTrackerTransformUpdatesAddListener");
    auto addListenerFunction = [this, InListener]
    {
        Listener = InListener;
    };

    if(previousTask.IsValid() && !previousTask.IsCompleted())
    {
        previousTask = UE::Tasks::Launch(taskName, MoveTemp(addListenerFunction), UE::Tasks::Prerequisites(previousTask));
    }
    else
    {
        previousTask = UE::Tasks::Launch(taskName, MoveTemp(addListenerFunction));
    }
}

void FTrackerTransformUpdates::RemoveListener(const TSharedRef<ITrackerTransformUpdateListener>& InListener)
{
    const auto taskName = TEXT("PhysicalObjectTrackerTransformUpdatesRemoveListener");
    auto removeListenerFunction = [this, InListener]
    {
        if (Listener == InListener)
        {
            Listener.Reset();
        }
    };

    if(previousTask.IsValid() && !previousTask.IsCompleted())
    {
        previousTask = UE::Tasks::Launch(taskName, MoveTemp(removeListenerFunction), UE::Tasks::Prerequisites(previousTask));
    }
    else
    {
        previousTask = UE::Tasks::Launch(taskName, MoveTemp(removeListenerFunction));
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