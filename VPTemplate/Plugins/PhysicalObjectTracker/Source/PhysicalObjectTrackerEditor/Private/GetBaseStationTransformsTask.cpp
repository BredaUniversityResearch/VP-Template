#include "GetBaseStationTransformsTask.h"

#include "PhysicalObjectTrackingUtility.h"

FGetBaseStationTransformsTask::FGetBaseStationTransformsTask(
	int32 InTargetNumBaseStationTransforms,
	const TMap<int32, FTransform>* InCalibratedBaseStationTransforms)
	:
TargetNumBaseStationTransforms(InTargetNumBaseStationTransforms),
CalibratedBaseStationTransforms(InCalibratedBaseStationTransforms)
{
	check(InCalibratedBaseStationTransforms != nullptr)
}

void FGetBaseStationTransformsTask::Tick(float DeltaTime)
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

bool FGetBaseStationTransformsTask::IsComplete() const
{
	return HasAcquiredTransforms;
}

TMap<int32, FTransform>& FGetBaseStationTransformsTask::GetResults()
{
	check(HasAcquiredTransforms)
	return BaseStationResults;
}

void FGetBaseStationTransformsTask::TakeBaseStationSamples()
{
	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);
	if (baseStationIds.IsEmpty()) { return; }

	for(int32 id: baseStationIds)
	{
		//Only sample base stations that are not yet calibrated.
		if(!CalibratedBaseStationTransforms->Contains(id))
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
}

bool FGetBaseStationTransformsTask::HasCompleteBaseStationsHistory()
{
	if(BaseStationTransforms.Num() + CalibratedBaseStationTransforms->Num() < TargetNumBaseStationTransforms)
	{
		return false;
	}

	//For each base station that currently has been detected, check if we have
	//gathered a full samples history for at least the target number of base stations.
	int currentCompleteBaseStationOffsets = 0;
	for(const auto& baseStation : BaseStationTransforms)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			++currentCompleteBaseStationOffsets;
		}
	}
	return (currentCompleteBaseStationOffsets + CalibratedBaseStationTransforms->Num() >= TargetNumBaseStationTransforms);
}

void FGetBaseStationTransformsTask::BuildBaseStationResults()
{
	check(BaseStationTransforms.Num() + CalibratedBaseStationTransforms->Num() >= TargetNumBaseStationTransforms)
	for(const auto& baseStation : BaseStationTransforms)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			BaseStationResults.Add(baseStation.Key, baseStation.Value.GetAveragedTransform(0.5f));
		}
	}
}