#include "TrackerTransformUpdates.h"

void FTrackerTransformUpdates::Update(
    TObjectPtr<UPhysicalObjectTrackingComponent> Component, 
    const FTransformUpdate& Update)
{
    previousTask = UE::Tasks::Launch(
        TEXT("TrackerTransformUpdate"),
        [*this, &Component, &Update]
        {
            if (Listener.IsValid()) { Listener->OnUpdate(Component, Update); }
        },
        UE::Tasks::Prerequisites(previousTask));
}

void FTrackerTransformUpdates::AddListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener)
{
    Listener = InListener;
}

void FTrackerTransformUpdates::RemoveListener(const TSharedPtr<ITrackerTransformUpdateListener>& InListener)
{
  if(Listener == InListener)
  {
      Listener.Reset();
  }
}

bool FTrackerTransformUpdates::HasListener() const
{
    return Listener.IsValid();
}

FTransformUpdate::FTransformUpdate(
    const FTimecode& InTimeCode, 
    const FTransform& InTransform)
        :
TimeCode(InTimeCode),
Transformation(InTransform)
{}

ITrackerTransformUpdateListener::~ITrackerTransformUpdateListener() = default;