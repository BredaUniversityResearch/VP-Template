#include "GetTrackerStaticTransformTask.h"

#include "PhysicalObjectTrackingUtility.h"

#include "SteamVRFunctionLibrary.h"

FGetTrackerStaticTransformTask::FGetTrackerStaticTransformTask(int32 a_TargetTrackerId, int32 MinStaticBaseStationOffsets)
	: TargetTrackerId(a_TargetTrackerId),
	MinBaseStationResults(MinStaticBaseStationOffsets),
	TransformHistory(SampleSizeSeconds * SamplesPerSecond)
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

		TransformHistory.TakeSample(TargetTrackerId);
		TakeBaseStationSamples();

		if(	TransformHistory.HasCompleteHistory() && 
			TransformHistory.GetAverageVelocity() < AverageVelocityThreshold &&
			HasCompleteBaseStationsHistoryAndBelowVelocityThreshold(AverageVelocityThreshold))
		{
			HasAcquiredTransformsAndOffsets = true;
			Result = TransformHistory.GetAveragedTransform();
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
	return Result;
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

	if(baseStationIds.IsEmpty())
	{
		//Dont sample if no base stations could be found.
		return;
	}

	for(int32 id : baseStationIds)
	{
		FTrackerTransformHistory& samples = BaseStationOffsets.FindOrAdd(id, { SampleSizeSeconds * SamplesPerSecond });

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
	if (BaseStationOffsets.Num() < MinBaseStationResults)
	{
		return false;
	}

	//For each base station that currently has been seen, check if at least
	//we have gathered a full sample history that has an average velocity below the threshold for at least the minimum number of base stations.
	int currentCompleteStaticBaseStationOffsets = 0;
	for(const auto& baseStation : BaseStationOffsets)
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
	check(BaseStationOffsets.Num() >= MinBaseStationResults);

	for(const auto& baseStation : BaseStationOffsets)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			const FTransform baseStationTransform = baseStation.Value.GetAveragedTransform();
			BaseStationResults.Add(baseStation.Key, baseStationTransform);
		}
	}
	check(BaseStationResults.Num() >= MinBaseStationResults);

}
