#include "PhysicalObjectTrackingComponent.h"

#include "PhysicalObjectTracker.h"
#include "PhysicalObjectTrackerSerialId.h"
#include "PhysicalObjectTrackingFilterSettings.h"
#include "PhysicalObjectTrackingReferencePoint.h"
#include "PhysicalObjectTrackingUtility.h"
#include "SteamVRFunctionLibrary.h"
#include "SteamVRInputDeviceFunctionLibrary.h"

#include"Engine/EngineTypes.h"

UPhysicalObjectTrackingComponent::UPhysicalObjectTrackingComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	bTickInEditor = true;
	bAutoActivate = true;
}

void UPhysicalObjectTrackingComponent::OnRegister()
{
	Super::OnRegister();
	if (FilterSettings != nullptr)
	{
		FilterSettings->OnFilterSettingsChanged.AddUObject(this, &UPhysicalObjectTrackingComponent::OnFilterSettingsChangedCallback);
	}
	RefreshDeviceId();
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
	
}

void UPhysicalObjectTrackingComponent::TickComponent(float DeltaTime, ELevelTick Tick,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);
	if (TrackerSerialId == nullptr) { return; }

	if (CurrentTargetDeviceId == -1)
	{	
		//TODO: Debug if this is actually trying to acquire in an interval. 
		//Suspect it to check if timer keeps under the interval when delta time is added and if it happens to go above it stops acquiring.
		DeviceIdAcquireTimer += DeltaTime;
		if (DeviceIdAcquireTimer < DeviceReacquireInterval) 
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
		FTransform trackerFromReference;
		if (TrackingSpaceReference != nullptr)
		{
			trackerFromReference = TrackingSpaceReference->ApplyTransformation(trackedPosition, trackedOrientation);
		}
		else
		{
			trackerFromReference = FTransform(trackedOrientation, trackedPosition);
		}

		if (WorldReferencePoint != nullptr)
		{
			trackerFromReference.SetLocation(WorldReferencePoint->GetActorTransform().TransformPosition(trackerFromReference.GetLocation()));
			//trackerFromReference.SetLocation((WorldReferencePoint->GetActorTransform().GetRotation() * trackerFromReference.GetLocation()) + WorldReferencePoint->GetActorTransform().GetLocation());
			trackerFromReference.SetRotation(WorldReferencePoint->GetActorTransform().TransformRotation(trackerFromReference.GetRotation()));
		}

		m_TransformHistory.AddSample(trackerFromReference);
		const FTransform filteredTransform = m_TransformHistory.GetAveragedTransform(FilterSettings);

		if(HasTransformationTargetComponent && TransformationTargetComponent.IsValid())
		{
			TransformationTargetComponent.Get()->SetWorldTransform(filteredTransform);
		}
		else
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
		if (PropertyChangedEvent.MemberProperty->HasMetaData("DeviceSerialId"))
		{
			RefreshDeviceId();
			if (CurrentTargetDeviceId == -1)
			{
				DeviceIdAcquireTimer = 0.0f;
			}
		}
		else if (PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("FilterSettings")))
		{
			FilterSettingsChangedHandle.Reset();
			if (FilterSettings != nullptr)
			{
				FilterSettings->OnFilterSettingsChanged.AddUObject(this, &UPhysicalObjectTrackingComponent::OnFilterSettingsChangedCallback);
			}
		}
		else if (PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("TransformationTargetComponentReference")))
		{
			if (HasTransformationTargetComponent)
			{
				UActorComponent* transformationTargetComponent = TransformationTargetComponentReference.GetComponent(GetOwner());
				if (transformationTargetComponent != nullptr)
				{
					TransformationTargetComponent = Cast<USceneComponent>(transformationTargetComponent);
					if (!TransformationTargetComponent.IsValid())
					{
						GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red,
							FString::Format(TEXT("PhysicalObjectTrackingComponent does not reference a component that is or inherits from a scenecomponent as movement target component. Component in actor: \"{0}\""),
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
	}
}
#endif

void UPhysicalObjectTrackingComponent::RefreshDeviceId()
{
	if (TrackerSerialId == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red,
			FString::Format(TEXT("PhysicalObjectTrackingComponent is refreshing the device id without a TrackerSerialId referenced on object \"{0}\""),
				FStringFormatOrderedArguments({ GetOwner()->GetName() })));
		return;
	}

	int32 foundDeviceId;
	if (FPhysicalObjectTrackingUtility::FindDeviceIdFromSerialId(TrackerSerialId->SerialId, foundDeviceId))
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
