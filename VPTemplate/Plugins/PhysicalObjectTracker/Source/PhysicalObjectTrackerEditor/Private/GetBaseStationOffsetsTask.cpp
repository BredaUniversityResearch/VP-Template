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
	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);
	if (baseStationIds.IsEmpty()) { return; }

	const FQuat baseStationRotationFix = FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(90.0f));
	FTransform relativeTrackerTransform = FTransform::Identity;

	{
		FVector trackerLocation;
		FQuat trackerRotation;
		if (!FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(TargetTrackerId, trackerLocation, trackerRotation))
		{
			return;
		}
		const FQuat trackerRotationInverse = trackerRotation.Inverse();

		//Determine the current position of the tracker relative to the calibrated base stations.
		FVector averagedCurrentTrackerLocation = FVector::ZeroVector;
		FVector	averagedCurrentTrackerRotationEuler = FVector::ZeroVector;
		int numCurrentBaseStationOffsetSamples = 0;

		for (const auto& initialBaseOffset : CalibratedBaseStationOffsets)
		{
			FVector baseStationLocation;
			FQuat baseStationRotation;
			if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(initialBaseOffset.Key, baseStationLocation, baseStationRotation))
			{
				//Get the translation between the tracker and the base station and reverse the rotation that is applied to it.
				const FVector trackerToBaseCurrentTranslation = trackerRotationInverse * (baseStationLocation - trackerLocation);
				const FQuat trackerToBaseCurrentRotation = (baseStationRotation * baseStationRotationFix) * trackerRotationInverse;

				averagedCurrentTrackerLocation += (initialBaseOffset.Value.GetLocation() - trackerToBaseCurrentTranslation);
				averagedCurrentTrackerRotationEuler += (trackerToBaseCurrentRotation * initialBaseOffset.Value.GetRotation().Inverse()).Euler();
				++numCurrentBaseStationOffsetSamples;
			}
		}

		if (numCurrentBaseStationOffsetSamples > 1)
		{
			averagedCurrentTrackerLocation /= numCurrentBaseStationOffsetSamples;
			averagedCurrentTrackerRotationEuler /= numCurrentBaseStationOffsetSamples;
			relativeTrackerTransform = FTransform(
				FQuat::MakeFromEuler(averagedCurrentTrackerRotationEuler),
				averagedCurrentTrackerLocation);
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

				const FVector baseToOriginTranslation = relativeTrackerTransform.GetLocation() + trackerToBaseStationTranslation;
				const FQuat baseToOriginRotation = relativeTrackerTransform.GetRotation() * trackerToBaseStationRotation;

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