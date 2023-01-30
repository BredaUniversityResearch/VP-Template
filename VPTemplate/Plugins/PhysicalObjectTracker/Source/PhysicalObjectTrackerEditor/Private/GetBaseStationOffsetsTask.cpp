#include "GetBaseStationOffsetsTask.h"

#include "PhysicalObjectTrackingUtility.h"

FGetBaseStationOffsetsTask::FGetBaseStationOffsetsTask(
	int32 InTargetTrackerId,
	int32 InTargetNumBaseStationTransforms,
	const FTransform& InTargetTrackerCalibrationTransform,
	const TMap<int32, FTransform>* InCalibratedBaseStationTransforms)
	:
TargetTrackerId(InTargetTrackerId),
TargetNumBaseStationTransforms(InTargetNumBaseStationTransforms),
TargetTrackerCalibrationTransform(InTargetTrackerCalibrationTransform),
CalibratedBaseStationTransforms(InCalibratedBaseStationTransforms)
{
	check(InCalibratedBaseStationTransforms != nullptr)
}

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
	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);
	if (baseStationIds.IsEmpty()) { return; }

	FVector trackerLocation;
	FQuat trackerRotation;
	if(!FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(TargetTrackerId, trackerLocation, trackerRotation))
	{
		return;	//If the target tracker could not be tracker, we can not get the relative offsets to the base stations so no need to sample the base stations and simply return.
	}

	const FTransform trackerTransform = FTransform(trackerRotation, trackerLocation) * TargetTrackerCalibrationTransform.Inverse();

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
				const FTransform baseStationTransform = FTransform(baseStationRotation, baseStationLocation);
				const FTransform originToBaseStation = trackerTransform * baseStationTransform;
				samples.AddSample(originToBaseStation);
			}
		}
	}
}

bool FGetBaseStationOffsetsTask::HasCompleteBaseStationsHistory()
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

void FGetBaseStationOffsetsTask::BuildBaseStationResults()
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