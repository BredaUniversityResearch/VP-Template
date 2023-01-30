#include "GetTrackerStaticTransformTask.h"

#include "PhysicalObjectTrackingUtility.h"

#include "SteamVRFunctionLibrary.h"

FGetTrackerStaticTransformTask::FGetTrackerStaticTransformTask(int32 a_TargetTrackerId, int32 MinStaticBaseStationOffsets)
	: TargetTrackerId(a_TargetTrackerId),
	MinBaseStationResults(MinStaticBaseStationOffsets),
	TrackerTransforms(SampleSizeSeconds * SamplesPerSecond)
{
}

void FGetTrackerStaticTransformTask::Tick(float DeltaTime)
{
	if (HasAcquiredTransformsAndOffsets)
	{
		return;
	}

	SampleDeltaTimeAccumulator += DeltaTime;
	constexpr float SecondsBetweenSamples = 1.0f / static_cast<float>(SamplesPerSecond);
	if (SampleDeltaTimeAccumulator > SecondsBetweenSamples)
	{
		SampleDeltaTimeAccumulator -= SecondsBetweenSamples;

		TrackerTransforms.TakeSample(TargetTrackerId);
		TakeBaseStationSamples();

		if(	TrackerTransforms.HasCompleteHistory() && 
			TrackerTransforms.GetAverageVelocity() < AverageVelocityThreshold &&
			HasCompleteBaseStationsHistoryAndBelowVelocityThreshold(AverageVelocityThreshold))
		{
			HasAcquiredTransformsAndOffsets = true;
			TrackerResult = TrackerTransforms.GetAveragedTransform(0.5f);
			BuildStaticBaseStationResults();
		}
	}
}

bool FGetTrackerStaticTransformTask::IsComplete() const
{
	return HasAcquiredTransformsAndOffsets;
}

const FTransform& FGetTrackerStaticTransformTask::GetResult() const
{
	check(HasAcquiredTransformsAndOffsets);
	return TrackerResult;
}

TMap<int32, FTransform>& FGetTrackerStaticTransformTask::GetBaseStationResults()
{
	check(HasAcquiredTransformsAndOffsets);
	return BaseStationResults;
}

void FGetTrackerStaticTransformTask::TakeBaseStationSamples()
{
	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);

	for(int32 id : baseStationIds)
	{
		FTrackerTransformHistory& samples = BaseStationTransforms.FindOrAdd(id, { SampleSizeSeconds * SamplesPerSecond });

		FVector baseStationLocation;
		FQuat baseStationRotation;
		if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(id, baseStationLocation, baseStationRotation))
		{
			samples.AddSample(FTransform(baseStationRotation, baseStationLocation));
		}
	}
}

bool FGetTrackerStaticTransformTask::HasCompleteBaseStationsHistoryAndBelowVelocityThreshold(float Threshold) const
{
	if (BaseStationTransforms.Num() < MinBaseStationResults)
	{
		return false;
	}

	//For each base station that currently has been seen, check if at least
	//we have gathered a full sample history that has an average velocity below the threshold for at least the minimum number of base stations.
	int currentCompleteStaticBaseStationOffsets = 0;
	for(const auto& baseStation : BaseStationTransforms)
	{
		if(baseStation.Value.HasCompleteHistory() && baseStation.Value.GetAverageVelocity() < Threshold)
		{
			++currentCompleteStaticBaseStationOffsets;
		}
	}
	return (currentCompleteStaticBaseStationOffsets >= MinBaseStationResults);
}

void FGetTrackerStaticTransformTask::BuildStaticBaseStationResults()
{
	check(BaseStationTransforms.Num() >= MinBaseStationResults);

	for(const auto& baseStation : BaseStationTransforms)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			BaseStationResults.Add(baseStation.Key, baseStation.Value.GetAveragedTransform(0.5f));
		}
	}
}
