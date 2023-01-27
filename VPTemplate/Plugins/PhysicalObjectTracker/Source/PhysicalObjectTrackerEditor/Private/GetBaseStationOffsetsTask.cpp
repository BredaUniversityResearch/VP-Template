#include "GetBaseStationOffsetsTask.h"

#include "PhysicalObjectTrackingUtility.h"

FGetBaseStationOffsetsTask::FGetBaseStationOffsetsTask(
	int32 InTargetTrackerId,
	int32 InTargetNumBaseStationOffsets,
	const FTransform& InTargetTrackerNeutralOffsetToSteamVROrigin,
	const TMap<int32, FTransform>& InCalibratedBaseStationOffsets)
	:
TargetTrackerId(InTargetTrackerId),
TargetNumBaseStationOffsets(InTargetNumBaseStationOffsets),
TargetTrackerNeutralOffsetToSteamVROrigin(InTargetTrackerNeutralOffsetToSteamVROrigin),
CalibratedBaseStationOffsets(InCalibratedBaseStationOffsets)
{}

void FGetBaseStationOffsetsTask::Tick(float DeltaTime)
{
	if(HasAcquiredTransforms)
	{
		return;
	}

	SampleDeltaTimeAccumulator += DeltaTime;
	constexpr float SecondsBetweenSamples = 1.f / static_cast<float>(SamplesPerSecond);
	if(SampleDeltaTimeAccumulator >= SecondsBetweenSamples)
	{
		SampleDeltaTimeAccumulator -= SecondsBetweenSamples;

		TakeBaseStationSamples();

		if(HasCompleteBaseStationsHistory())
		{
			HasAcquiredTransforms = true;
			BuildBaseStationResults();
		}
	}
}

bool FGetBaseStationOffsetsTask::IsComplete() const
{
	return HasAcquiredTransforms;
}

TMap<int32, FTransform>& FGetBaseStationOffsetsTask::GetResults()
{
	check(HasAcquiredTransforms)
	return BaseStationResults;
}

void FGetBaseStationOffsetsTask::TakeBaseStationSamples()
{
	//- Get the current offset to a known base station
	//- Get the difference in the current and initial offset to the known base station to determine the relative position of the tracker
	//- Get the current offset to a new base station

	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);
	if (baseStationIds.IsEmpty()) { return; }

	const FQuat baseStationRotationFix = FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(90.0f));

	FTransform relativeTrackerTransform = FTransform::Identity;
	{
		FTransform trackerTransform = FTransform::Identity;
		{
			FVector trackerLocation;
			FQuat trackerRotation;
			if(!FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(TargetTrackerId, trackerLocation, trackerRotation))
			{
				return;	//If the target tracker could not be tracker, we can not get the relative offsets to the base stations so no need to sample the base stations and simply return.
			}
			
			trackerTransform = FPhysicalObjectTrackingUtility::FixTrackerTransform(FTransform(trackerRotation, trackerLocation));
		}

		//Determine the current position of the tracker relative to the already calibrated base stations.
		FVector averageOriginToTrackerTranslation = FVector::ZeroVector;
		FVector	averageOriginToTrackerRotationEuler = FVector::ZeroVector;
		int numCurrentBaseStationOffsetSamples = 0;

		for (const auto& initialBaseOffset : CalibratedBaseStationOffsets)
		{
			FVector baseStationLocation;
			FQuat baseStationRotation;
			if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(initialBaseOffset.Key, baseStationLocation, baseStationRotation))
			{
				const FTransform baseStationTransform = FTransform(baseStationRotation, baseStationLocation);
				const FTransform trackerToBaseStation = baseStationTransform * trackerTransform.Inverse();
				const FTransform originToTracker = initialBaseOffset.Value * trackerToBaseStation.Inverse();

				if (GEditor && GEditor->GetEditorWorldContext().World())
				{
					const FVector arrowStart = trackerToBaseStation.TransformPosition(FVector::ZeroVector);
					const FVector arrowEnd = trackerToBaseStation.TransformPosition(FVector::ForwardVector * 100.f);
					DrawDebugDirectionalArrow(GEditor->GetEditorWorldContext().World(), arrowStart, arrowEnd, 10.f, FColor::Orange, false, 2, 0, 1.f);
				}

				averageOriginToTrackerTranslation += originToTracker.GetTranslation();
				averageOriginToTrackerRotationEuler += originToTracker.GetRotation().Euler();
				++numCurrentBaseStationOffsetSamples;
			}
			//TODO: temporarily only take one sample and do not average as I am not sure if the averaging works. (Although I am also not sure if the relative calculation works).
			if (numCurrentBaseStationOffsetSamples == 1) { break; }
		}

		if (numCurrentBaseStationOffsetSamples >= 1)
		{
			averageOriginToTrackerTranslation = averageOriginToTrackerTranslation / numCurrentBaseStationOffsetSamples;
			averageOriginToTrackerRotationEuler = averageOriginToTrackerRotationEuler / numCurrentBaseStationOffsetSamples;
			relativeTrackerTransform = FTransform(FQuat::MakeFromEuler(averageOriginToTrackerRotationEuler), averageOriginToTrackerTranslation);
		}
		else
		{
			//If no calibrated base station could be tracked, don't sample.
			return;
		}

	}

	for(int32 id: baseStationIds)
	{
		//Only sample base stations that are not yet calibrated.
		if(!CalibratedBaseStationOffsets.Contains(id))
		{
			FTrackerTransformHistory& samples = BaseStationOffsets.FindOrAdd(id, { SampleSizeSeconds * SamplesPerSecond });

			FVector baseStationLocation;
			FQuat baseStationRotation;
			if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(id, baseStationLocation, baseStationRotation))
			{
				const FVector trackerToBaseStationTranslation = relativeTrackerTransform.GetRotation().Inverse() * (baseStationLocation - relativeTrackerTransform.GetLocation());
				const FQuat trackerToBaseStationRotation = (baseStationRotation * baseStationRotationFix) * relativeTrackerTransform.GetRotation().Inverse();

				//The relative tracker transform and tracker to base transform should be in the same space (reference point space)
				const FVector baseToOriginTranslation = relativeTrackerTransform.GetLocation() + trackerToBaseStationTranslation;
				const FQuat baseToOriginRotation = trackerToBaseStationRotation * relativeTrackerTransform.GetRotation();

				samples.AddSample(FTransform(baseToOriginRotation, baseToOriginTranslation));
			}
		}
	}
}

bool FGetBaseStationOffsetsTask::HasCompleteBaseStationsHistory()
{
	if(BaseStationOffsets.Num() + CalibratedBaseStationOffsets.Num() < TargetNumBaseStationOffsets)
	{
		return false;
	}

	//For each base station that currently has been detected, check if we have
	//gathered a full samples history for at least the target number of base stations.
	int currentCompleteBaseStationOffsets = 0;
	for(const auto& baseStation : BaseStationOffsets)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			++currentCompleteBaseStationOffsets;
		}
	}
	return (currentCompleteBaseStationOffsets + CalibratedBaseStationOffsets.Num() >= TargetNumBaseStationOffsets);
}

void FGetBaseStationOffsetsTask::BuildBaseStationResults()
{
	check(BaseStationOffsets.Num() + CalibratedBaseStationOffsets.Num() >= TargetNumBaseStationOffsets)
	for(const auto& baseStation : BaseStationOffsets)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			BaseStationResults.Add(baseStation.Key, baseStation.Value.GetAveragedTransform());
		}
	}
}