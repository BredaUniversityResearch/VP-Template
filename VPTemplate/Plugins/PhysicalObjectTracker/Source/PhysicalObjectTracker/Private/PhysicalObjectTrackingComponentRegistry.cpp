#include "PhysicalObjectTrackingComponentRegistry.h"
#include "Delegates/Delegate.h"

void FPhysicalObjectTrackingComponentRegistry::AddComponent(const TObjectPtr<UPhysicalObjectTrackingComponent>& Component)
{
    CurrentObjectTrackingComponents.Add(Component);
    OnComponentRegistered.Broadcast(Component);
}

void FPhysicalObjectTrackingComponentRegistry::RemoveComponent(const TObjectPtr<UPhysicalObjectTrackingComponent>& Component)
{
    CurrentObjectTrackingComponents.Remove(Component);
    OnComponentUnregistered.Broadcast(Component);
}

FDelegateHandle FPhysicalObjectTrackingComponentRegistry::AddOnComponentRegisteredDelegate(
    const FOnPhysicalObjectTrackingComponentRegistered::FDelegate& Delegate)
{
    return OnComponentRegistered.Add(Delegate);
}

FDelegateHandle FPhysicalObjectTrackingComponentRegistry::AddOnComponentUnregisteredDelegate(
    const FOnPhysicalObjectTrackingComponentUnregistered::FDelegate& Delegate)
{
    return OnComponentUnregistered.Add(Delegate);
}

const TSet<TObjectPtr<UPhysicalObjectTrackingComponent>>& FPhysicalObjectTrackingComponentRegistry::GetCurrentObjectTrackingComponents() const
{
    return CurrentObjectTrackingComponents;
}
