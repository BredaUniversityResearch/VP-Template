#include "PhysicalObjectTrackingComponent.h"

#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackerSerialId.h"
#include "PhysicalObjectTrackingFilterSettings.h"
#include "PhysicalObjectTrackingReferencePoint.h"
#include "PhysicalObjectTrackingUtility.h"
#include "SteamVRFunctionLibrary.h"
#include "SteamVRInputDeviceFunctionLibrary.h"

#include"Engine/EngineTypes.h"
#include "Engine/TimecodeProvider.h"

UPhysicalObjectTrackingComponent::UPhysicalObjectTrackingComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	bTickInEditor = true;
	bAutoActivate = true;
}

void UPhysicalObjectTrackingComponent::OnRegister()
{
	Super::OnRegister();

	//Should never fail as this is in the same module.
	//TODO: check if there is a function that returns the current module instead of using string lookup for the module.
	const FPhysicalObjectTracker& trackerModule = FModuleManager::Get().GetModuleChecked<FPhysicalObjectTracker>("PhysicalObjectTracker");
	trackerModule.OnTrackingComponentRegistered.Broadcast(AsShared());

	if (FilterSettings != nullptr)
	{
		FilterSettingsChangedHandle.Reset();      
		FilterSettingsChangedHandle = FilterSettings->OnFilterSettingsChanged.AddUObject(this, &UPhysicalObjectTrackingComponent::OnFilterSettingsChangedCallback);
	}
	if (TrackerSerialIdAsset != nullptr)
	{
		SerialIdChangedHandle.Reset();
		SerialIdChangedHandle = TrackerSerialIdAsset->OnSerialIdChanged.AddUObject(this, &UPhysicalObjectTrackingComponent::OnTrackerSerialIdChangedCallback);
		RefreshDeviceId();
	}
	if(TrackingSpaceReference != nullptr)
	{
		TrackingSpaceReference->UpdateRuntimeDataIfNeeded();
	}

	ExtractTransformationTargetComponentReferenceIfValid();
	OnFilterSettingsChangedCallback();
}

void UPhysicalObjectTrackingComponent::BeginPlay()
{
	Super::BeginPlay();
	if (TrackingSpaceReference == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.0f, FColor::Red, 
			FString::Format(TEXT("PhysicalObjectTrackingComponent \"{0}\" does not have reference a tracking space on object \"{1}\""), 
				FStringFormatOrderedArguments({GetName(),  GetOwner()->GetName() })));
	}
	else
	{
		TrackingSpaceReference->UpdateRuntimeDataIfNeeded();
	}
}

void UPhysicalObjectTrackingComponent::TickComponent(float DeltaTime, ELevelTick Tick,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);
	if (TrackerSerialIdAsset == nullptr) { return; }

	if (CurrentTargetDeviceId == -1)
	{
		DeviceIdAcquireTimer += DeltaTime;
		if (DeviceIdAcquireTimer > DeviceReacquireInterval) 
		{
			RefreshDeviceId();
			DeviceIdAcquireTimer -= DeviceReacquireInterval;
		}
		return;
	}

	FVector trackedPosition;
	FQuat trackedOrientation;	
	if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(CurrentTargetDeviceId, trackedPosition, trackedOrientation))
	{
		FTransform trackerFromReference(trackedOrientation, trackedPosition);
		if (TrackingSpaceReference != nullptr)
		{
			trackerFromReference = TrackingSpaceReference->GetTrackerReferenceSpaceTransform(trackerFromReference);
			FTimecode currentTimeCode = FApp::GetTimecode();
			OnTrackerTransformUpdate.Broadcast(*this, currentTimeCode, trackerFromReference); //TODO: maybe change this from being a delegate to a buffer for better performance (async).
			//GEngine->GetTimecodeProvider(); TODO: Seems like FApp::GetTimecode uses this under the hood but it might be necessary to use it directly?
		}

		if (WorldReferencePoint != nullptr)
		{
			trackerFromReference.SetLocation(WorldReferencePoint->GetActorTransform().TransformPosition(trackerFromReference.GetLocation()));
			trackerFromReference.SetRotation(WorldReferencePoint->GetActorTransform().TransformRotation(trackerFromReference.GetRotation()));
		}

		m_TransformHistory.AddSample(trackerFromReference);
		const FTransform filteredTransform = m_TransformHistory.GetAveragedTransform(FilterSettings);

		if(HasTransformationTargetComponent && TransformationTargetComponent != nullptr)
		{
			TransformationTargetComponent.Get()->SetWorldTransform(filteredTransform);
		}
		else if(!HasTransformationTargetComponent)
		{
			GetOwner()->SetActorTransform(filteredTransform);
		}
	}
	else
	{
		DebugCheckIfTrackingTargetExists();
		//UE_LOG(LogPhysicalObjectTracker, Warning, TEXT("Failed to acquire TrackedDevicePosition for device id %i"), CurrentTargetDeviceId);
	}

}

#if WITH_EDITOR
void UPhysicalObjectTrackingComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty != nullptr)
	{
		//TODO: check if this event is enough to update device id if serial id changes
		//either by being set in the property or the data asset being changed.
		if (PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("TrackerSerialId")))
		{
			SerialIdChangedHandle.Reset();
			if (TrackerSerialIdAsset != nullptr)
			{
				SerialIdChangedHandle = TrackerSerialIdAsset->OnSerialIdChanged.AddUObject(this, &UPhysicalObjectTrackingComponent::OnTrackerSerialIdChangedCallback);
				OnTrackerSerialIdChangedCallback();
			}
		}
		else if (PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("FilterSettings")))
		{
			FilterSettingsChangedHandle.Reset();
			if (FilterSettings != nullptr)
			{
				FilterSettingsChangedHandle = FilterSettings->OnFilterSettingsChanged.AddUObject(this, &UPhysicalObjectTrackingComponent::OnFilterSettingsChangedCallback);
			}
		}
		else if(PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("TrackingSpaceReference")))
		{
			if(TrackingSpaceReference != nullptr)
			{
				TrackingSpaceReference->UpdateRuntimeDataIfNeeded();
			}
		}
		else if (PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("TransformationTargetComponentReference")))
		{
			ExtractTransformationTargetComponentReferenceIfValid();
		}
	}
}
#endif

void UPhysicalObjectTrackingComponent::RefreshDeviceId()
{
	if (TrackerSerialIdAsset == nullptr || TrackerSerialIdAsset->GetSerialId().IsEmpty())
	{
		CurrentTargetDeviceId = -1;
		if(GetOwner() != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red,
				FString::Format(TEXT("PhysicalObjectTrackingComponent is refreshing the device id without a valid TrackerSerialId referenced on object \"{0}\""),
					FStringFormatOrderedArguments({ GetOwner()->GetName() })));
		}
		return;
	}

	int32 foundDeviceId;
	if (FPhysicalObjectTrackingUtility::FindDeviceIdFromSerialId(TrackerSerialIdAsset->GetSerialId(), foundDeviceId))
	{
		if (CurrentTargetDeviceId != foundDeviceId)
		{
			CurrentTargetDeviceId = foundDeviceId;
		}
	}
}

const FTransform* UPhysicalObjectTrackingComponent::GetWorldReferencePoint() const
{
	return WorldReferencePoint != nullptr ? &WorldReferencePoint->GetActorTransform() : nullptr;
}

const UPhysicalObjectTrackingReferencePoint* UPhysicalObjectTrackingComponent::GetTrackingReferencePoint() const
{
	return TrackingSpaceReference;
}

void UPhysicalObjectTrackingComponent::DebugCheckIfTrackingTargetExists() const
{
	TArray<int32> deviceIds{};
	USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::Controller, deviceIds);
	if (!deviceIds.Contains(CurrentTargetDeviceId))
	{
		TWideStringBuilder<4096> builder{};
		builder.Appendf(TEXT("Could not find SteamVR Controller with DeviceID: %i. Valid device IDs are: "), CurrentTargetDeviceId);
		for (int32 deviceId : deviceIds)
		{
			builder.Appendf(TEXT("%i, "), deviceId);
		}
		GEngine->AddOnScreenDebugMessage(565498, 0.0f, FColor::Red, builder.ToString(), false);
	}
}

void UPhysicalObjectTrackingComponent::OnFilterSettingsChangedCallback()
{
	m_TransformHistory.SetFromFilterSettings(FilterSettings);
}

void UPhysicalObjectTrackingComponent::OnTrackerSerialIdChangedCallback()
{
	RefreshDeviceId();
	if (CurrentTargetDeviceId == -1)
	{
		DeviceIdAcquireTimer = 0.0f;
	}
}

void UPhysicalObjectTrackingComponent::ExtractTransformationTargetComponentReferenceIfValid()
{
    AActor* owningActor =  GetOwner();
	if (HasTransformationTargetComponent && owningActor != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Blue, FString(TEXT("ExtractingComponentReference")));
		UActorComponent* transformationTargetActorComponent = TransformationTargetComponentReference.GetComponent(owningActor);
		if (transformationTargetActorComponent != nullptr)
		{
			TransformationTargetComponent = Cast<USceneComponent>(transformationTargetActorComponent);
			if (TransformationTargetComponent == nullptr)
			{
				GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red,
					FString::Format(TEXT("PhysicalObjectTrackingComponent does not reference a component that is or inherits from a scene component as movement target component. Component in actor: \"{0}\""),
						FStringFormatOrderedArguments({ GetOwner()->GetName() })));
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red,
				FString::Format(TEXT("PhysicalObjectTrackingComponent does not reference a valid component as movement target component. Component in actor: \"{0}\""),
					FStringFormatOrderedArguments({ GetOwner()->GetName() })));
		}
	}
}