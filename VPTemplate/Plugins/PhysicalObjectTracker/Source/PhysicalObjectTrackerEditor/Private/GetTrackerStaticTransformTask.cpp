#include "GetTrackerStaticTransformTask.h"

#include "PhysicalObjectTrackingUtility.h"

#include "SteamVRFunctionLibrary.h"

FGetTrackerStaticTransformTask::FGetTrackerStaticTransformTask(
	int32 a_TargetTrackerId, 
	int32 InMinStaticBaseStationOffsets)
	: TargetTrackerId(a_TargetTrackerId),
	MinStaticBaseStationOffsets(InMinStaticBaseStationOffsets),
	TransformHistory(SampleSizeSeconds * SamplesPerSecond)
{
}

void FGetTrackerStaticTransformTask::Tick(float DeltaTime)
{
	if (HasAcquiredTransform && HasAcquiredBaseStationOffsets)
	{
		return;
	}

	SampleDeltaTimeAccumulator += DeltaTime;
	constexpr float SecondsBetweenSamples = 1.0f / static_cast<float>(SamplesPerSecond);
	if (SampleDeltaTimeAccumulator > SecondsBetweenSamples)
	{
		SampleDeltaTimeAccumulator -= SecondsBetweenSamples;

		if (!HasAcquiredTransform)
		{
			TransformHistory.TakeSample(TargetTrackerId);
			if(	TransformHistory.HasCompleteHistory() && 
				TransformHistory.GetAverageVelocity() < AverageVelocityThreshold)
			{
				HasAcquiredTransform = true;
				Result = TransformHistory.GetAveragedTransform();
			}
		}
		else if (!HasAcquiredBaseStationOffsets)
		{
			TakeBaseStationSamples();
			if (HasCompleteBaseStationsHistoryAndBelowVelocityThreshold(AverageVelocityThreshold))
			{
				HasAcquiredBaseStationOffsets = true;
				BuildStaticBaseStationResults();
			}
		}
	}
}

bool FGetTrackerStaticTransformTask::IsComplete() const
{
	return HasAcquiredTransform && HasAcquiredBaseStationOffsets;
}

const FTransform& FGetTrackerStaticTransformTask::GetResult() const
{
	check(HasAcquiredTransform);
	return Result;
}

TMap<int32, FTransform>& FGetTrackerStaticTransformTask::GetBaseStationResults()
{
	check(HasAcquiredBaseStationOffsets);
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
	if (BaseStationOffsets.Num() < MinStaticBaseStationOffsets)
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
	return (currentCompleteStaticBaseStationOffsets >= MinStaticBaseStationOffsets);
}

void FGetTrackerStaticTransformTask::BuildStaticBaseStationResults()
{
	check(BaseStationOffsets.Num() >= MinStaticBaseStationOffsets);

	for(const auto& baseStation : BaseStationOffsets)
	{
		if(baseStation.Value.HasCompleteHistory())
		{
			static const FQuat BaseStationRotationFix = FQuat(FVector::RightVector, FMath::DegreesToRadians(0.0f));
			static const FMatrix DeviceToWorldSpace =
				FRotationMatrix::Make(FQuat(FVector::YAxisVector,
					FMath::DegreesToRadians(90))) * FScaleMatrix::Make(FVector(1.0f, -1.0f, -1.0f));

			const FTransform baseStationTransform = baseStation.Value.GetAveragedTransform();

			//Get the translation between the tracker and the base station and reverse the rotation that is applied to it.
			const FVector relativeTranslation = Result.GetRotation().Inverse().RotateVector(baseStationTransform.GetLocation() - Result.GetLocation());
			const FQuat relativeRotation = (baseStationTransform.GetRotation() * BaseStationRotationFix) * Result.GetRotation().Inverse();

			const FVector worldSpacePosition = DeviceToWorldSpace.TransformPosition(relativeTranslation);

			BaseStationResults.Add(baseStation.Key, FTransform(relativeRotation, worldSpacePosition));
		}
	}
}
