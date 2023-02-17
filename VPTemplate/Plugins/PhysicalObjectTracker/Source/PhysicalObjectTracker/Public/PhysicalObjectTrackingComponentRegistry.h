#pragma once

class UPhysicalObjectTrackingComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhysicalObjectTrackingComponentRegistered, TObjectPtr<UPhysicalObjectTrackingComponent>)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhysicalObjectTrackingComponentUnregistered, TObjectPtr<UPhysicalObjectTrackingComponent>)

class PHYSICALOBJECTTRACKER_API FPhysicalObjectTrackingComponentRegistry
{

public:

    void AddComponent(const TObjectPtr<UPhysicalObjectTrackingComponent>& Component);
    void RemoveComponent(const TObjectPtr<UPhysicalObjectTrackingComponent>& Component);

	FDelegateHandle AddOnComponentRegisteredDelegate(const FOnPhysicalObjectTrackingComponentRegistered::FDelegate& Delegate);
	FDelegateHandle AddOnComponentUnregisteredDelegate(const FOnPhysicalObjectTrackingComponentUnregistered::FDelegate& Delegate);

	const TSet<TObjectPtr<UPhysicalObjectTrackingComponent>>& GetCurrentObjectTrackingComponents() const;

private:

	FOnPhysicalObjectTrackingComponentRegistered OnComponentRegistered;
	FOnPhysicalObjectTrackingComponentUnregistered OnComponentUnregistered;

	TSet<TObjectPtr<UPhysicalObjectTrackingComponent>> CurrentObjectTrackingComponents;

};