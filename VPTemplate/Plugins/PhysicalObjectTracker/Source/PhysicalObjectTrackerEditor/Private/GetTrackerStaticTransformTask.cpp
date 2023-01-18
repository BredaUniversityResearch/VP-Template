#include "GetTrackerStaticTransformTask.h"

#include "PhysicalObjectTrackingUtility.h"

#include "SteamVRFunctionLibrary.h"

FGetTrackerStaticTransformTask::FGetTrackerStaticTransformTask(int32 a_TargetTrackerId)
	: m_TargetTrackerId(a_TargetTrackerId)
	, m_TransformHistory(SampleSizeSeconds * SamplesPerSecond)
{
}

void FGetTrackerStaticTransformTask::Tick(float DeltaTime)
{
	if (m_HasAcquiredTransform)
	{
		return;
	}

	//TODO:
	//Try to improve accuracy and consistency of the calibration.
	//Calculate the offsets to all the base stations when calibrating.
	//Store the offsets to all the base stations and use them to manually calculate the position of the tracker.
	//use USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation & USteamVRFunctionLibrary::GetValidTrackedDeviceIds with ESteamVRTrackedDeviceType::TrackingReference to get the transforms from the BaseStations.
	//When synching a tracker, it is put at a point in the real-life scene. That point has to represent the origin of digital scene.
	//This is currently done by using the SteamVR Function library and using GetTrackedDevicePositionAndOrientation, which returns the transformation relative to an arbitrary origin point internal to SteamVR.
	//The problem is that the internal origin point seems to not always be consistent throughout time, even in the same session.
	//To combat this problem, the internal original point of SteamVR should be mitigated.
	//The way to do this is to use relative offsets of the tracker to the base stations.
	//When getting the transformations of devices at any point in time, they are relative to the same internal SteamVR origin point.
	//This makes it possible to calculate offsets between each base station and tracker.
	//Then we can use the difference in offset to determine the transformation relative to the origin point of the scene.
	//When syncing a tracker: 
	//1. Get transformation relative to each base station (Require a minimum amount of base stations (4))
	//Note: -------------------------------------------------------------------------------------------
	//Might be possible to register all base stations (6) but when walking around, the tracker is not on the supposed origin point anymore and thus the tracker would already need to be tracked.
	// But without walking around it is not possible for all base stations to see the tracker on the origin point.
	// ------------------------------------------------------------------------------------------------
	//When tracking a tracker: 
	//2. Get transformation relative to each base station.
	//3. For each base station (currently connected), calculate the difference in relative offsets.
	//4. Average all of the differences in transformations to create 1 final transformation, which will become the sample.



	SampleDeltaTimeAccumulator += DeltaTime;
	constexpr float SecondsBetweenSamples = 1.0f / static_cast<float>(SamplesPerSecond);
	if (SampleDeltaTimeAccumulator > SecondsBetweenSamples)
	{
		SampleDeltaTimeAccumulator -= SecondsBetweenSamples;

		if(m_TransformHistory.TakeSample(m_TargetTrackerId))
		{
			TakeBaseStationSamples(m_TransformHistory.GetLatest());
		}

		if (m_TransformHistory.HasCompleteHistory() && HasCompleteBaseStationsHistory())
		{
			if (m_TransformHistory.GetAverageVelocity() < AverageVelocityThreshold)
			{
				m_HasAcquiredTransform = true;
				m_Result = m_TransformHistory.GetLatest();
				BuildStaticBaseStationResults();
			}
		}
	}
}

bool FGetTrackerStaticTransformTask::IsComplete() const
{
	return m_HasAcquiredTransform;
}

const FTransform& FGetTrackerStaticTransformTask::GetResult() const
{
	check(m_HasAcquiredTransform);
	return m_Result;
}

const TMap<int32, FTransform>& FGetTrackerStaticTransformTask::GetBaseStationResults() const
{
	check(m_HasAcquiredTransform);
	return m_BaseStationResults;
}

void FGetTrackerStaticTransformTask::TakeBaseStationSamples(const FTransform& TrackerTransform)
{
	TArray<int32> baseStationIds;
	FPhysicalObjectTrackingUtility::GetAllTrackingReferenceDeviceIds(baseStationIds);

	for(int32 id : baseStationIds)
	{
		FTrackerTransformHistory& samples = BaseStationOffsets.FindOrAdd(id, { SampleSizeSeconds * SamplesPerSecond });

		FVector baseStationLocation;
		FQuat baseStationRotation;
		if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(id, baseStationLocation, baseStationRotation))
		{
			//The tracker is assumed to be static at this point and thus the offsets to the base stations should simply be
			//the current base-station transform - the current tracker transform.
			const FTransform relativeTransformation = FPhysicalObjectTrackingUtility::GetRelativeTransform(
				TrackerTransform, FTransform(baseStationRotation, baseStationLocation));

			samples.AddSample(relativeTransformation);
		}
	}
}

bool FGetTrackerStaticTransformTask::HasCompleteBaseStationsHistory()
{
	if (BaseStationOffsets.Num() < MinStaticBaseStationOffsets)
	{
		return false;
	}

	//For each base station that currently has been seen, check if at least
	//we have gathered a full sample history for at least the minimum number of base stations.
	unsigned int currentCompleteStaticBaseStationOffsets = 0;
	for(const auto& baseStation : BaseStationOffsets)
	{
		if(baseStation.Value.HasCompleteHistory())
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
			m_BaseStationResults.Add(baseStation.Key, baseStation.Value.GetAveragedTransform());
		}
	}
}
