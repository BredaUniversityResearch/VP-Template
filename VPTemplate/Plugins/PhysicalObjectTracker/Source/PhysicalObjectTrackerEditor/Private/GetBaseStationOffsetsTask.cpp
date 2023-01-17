#include "GetBaseStationOffsetsTask.h"

#include "PhysicalObjectTrackingUtility.h"

FGetBaseStationOffsetsTask::FGetBaseStationOffsetsTask(
	int32 TargetTracker,
	int32 InTargetNumBaseStationOffsets,
	const TMap<int32, FTransform>* CompletedBaseStations)
	:
TargetTrackerId(TargetTracker),
TargetNumBaseStationOffsets(InTargetNumBaseStationOffsets),
CompletedBaseStationsIds(CompletedBaseStations)
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

	for(int32 id: baseStationIds)
	{
		if(!CompletedBaseStationsIds->Contains(id))
		{
			FTrackerTransformHistory& samples = BaseStationOffsets.FindOrAdd(id, { SampleSizeSeconds * SamplesPerSecond });

			FVector baseStationLocation;
			FQuat baseStationRotation;
			if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(id, baseStationLocation, baseStationRotation))
			{
				const FTransform relativeTransformation =  FPhysicalObjectTrackingUtility::GetRelativeTransform(
					trackerLocation, trackerRotation, 
					baseStationLocation, baseStationRotation);

				samples.AddSample(relativeTransformation);
			}
		}
	}
}

bool FGetBaseStationOffsetsTask::HasCompleteBaseStationsHistory()
{
	if(BaseStationOffsets.Num() + CompletedBaseStationsIds->Num() < TargetNumBaseStationOffsets)
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
	check(BaseStationOffsets.Num() + CompletedBaseStationsIds->Num() >= TargetNumBaseStationOffsets)

	for(const auto& baseStation : BaseStationOffsets)
	{
		check(baseStation.Value.HasCompleteHistory())
		BaseStationResults.Add(baseStation.Key, baseStation.Value.GetAveragedTransform());
	}
}
