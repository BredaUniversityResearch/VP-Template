#include "GetBaseStationOffsetsTask.h"

#include "PhysicalObjectTrackingUtility.h"

FGetBaseStationOffsetsTask::FGetBaseStationOffsetsTask(
	int32 InTargetTrackerId,
	int32 InTargetNumBaseStationOffsets,
	const FTransform& InTargetTrackerNeutralOffsetToSteamVROrigin,
	TMap<int32, FTransform>* InCalibratedBaseStationOffsets)
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

const TMap<int32, FTransform>& FGetBaseStationOffsetsTask::GetResults() const
{
	check(HasAcquiredTransforms);
	return BaseStationResults;
}

void FGetBaseStationOffsetsTask::TakeBaseStationSamples()
{
	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);

	FVector trackerLocation;
	FQuat trackerRotation;

	if (!FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(TargetTrackerId, trackerLocation, trackerRotation))
	{
		return;
	}

	//Determine the current position of the tracker relative to the calibrated base stations.
	FVector averagedCurrentTrackerLocation;
	FVector	averagedCurrentTrackerRotationEuler;
	int numCurrentBaseStationOffsetSamples = 0;
	for(const auto& initialBaseOffset : (*CalibratedBaseStationOffsets))
	{
		FVector baseStationLocation;
		FQuat baseStationRotation;
		if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(initialBaseOffset.Key, baseStationLocation, baseStationRotation))
		{
			FTransform currentOffset = FPhysicalObjectTrackingUtility::GetRelativeTransform(
				trackerLocation, trackerRotation, 
				baseStationLocation, baseStationRotation);
			averagedCurrentTrackerLocation += currentOffset.GetLocation();
			averagedCurrentTrackerRotationEuler += currentOffset.GetRotation().Euler();
			++numCurrentBaseStationOffsetSamples;
		}
	}

	if(numCurrentBaseStationOffsetSamples > 1)
	{
		averagedCurrentTrackerLocation /= numCurrentBaseStationOffsetSamples;
		averagedCurrentTrackerRotationEuler /= numCurrentBaseStationOffsetSamples;
	}
	else
	{
		return;
	}

	for(int32 id: baseStationIds)
	{
		//Only sample base stations that are not yet calibrated.
		if(!CalibratedBaseStationOffsets->Contains(id))
		{
			FTrackerTransformHistory& samples = BaseStationOffsets.FindOrAdd(id, { SampleSizeSeconds * SamplesPerSecond });


			FVector baseStationLocation;
			FQuat baseStationRotation;
			if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(id, baseStationLocation, baseStationRotation))
			{
				//Offset from the target tracker to the base station at the time of sampling (, when the tracker is moving)
				const FTransform currentRelativeTransformation =  FPhysicalObjectTrackingUtility::GetRelativeTransform(
					trackerLocation, trackerRotation, 
					baseStationLocation, baseStationRotation);

				//Calculate the offset from the neutral point (Led Volume Origin) to the base station.
				//1. Get the current transformation of the target tracker relative to the already calibrated base stations.
				//2. Add the relative offset to the base station to the trackers position.
				const FTransform baseStationOffsetToNeutral =

				samples.AddSample(relativeTransformation);
			}
		}
	}
}

bool FGetBaseStationOffsetsTask::HasCompleteBaseStationsHistory()
{
	if(BaseStationOffsets.Num() + CalibratedBaseStationOffsets->Num() < TargetNumBaseStationOffsets)
	{
		return false;
	}

	//For each base station that currently has been detected, check if we have
	//gathered a full samples history for at least the target number of base stations.
	unsigned int currentCompleteBaseStationOffsets = 0;
	for(const auto& baseStation : BaseStationOffsets)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			++currentCompleteBaseStationOffsets;
		}
	}
	return (currentCompleteBaseStationOffsets >= TargetNumBaseStationOffsets);
}

void FGetBaseStationOffsetsTask::BuildBaseStationResults()
{
	check(BaseStationOffsets.Num() + CalibratedBaseStationOffsets->Num() >= TargetNumBaseStationOffsets)

	for(const auto& baseStation : BaseStationOffsets)
	{
		check(baseStation.Value.HasCompleteHistory())
		BaseStationResults.Add(baseStation.Key, baseStation.Value.GetAveragedTransform());
	}
}
