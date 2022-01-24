#include "TrackerTransformHistory.h"

#include "PhysicalObjectTrackingUtility.h"

FTrackerTransformHistory::FTrackerTransformHistory(int32 a_SampleCount, int32 a_TrackerId)
	: m_TargetSampleCount(a_SampleCount)
	, m_TargetTrackerId(a_TrackerId)
	, m_History(a_SampleCount)
{
}

void FTrackerTransformHistory::TakeSample()
{
	FVector trackerPosition;
	FQuat trackerRotation;
	if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(m_TargetTrackerId, trackerPosition, trackerRotation))
	{
		m_History.Add(FTransform(trackerRotation, trackerPosition));
		while (m_History.Num() > m_TargetSampleCount)
		{
			m_History.PopFrontValue();
		}
	}
	else
	{
		m_History.Reset();
	}

	if (m_History.Num() == m_TargetSampleCount)
	{
		auto it = m_History.begin();
		FVector firstPosition = it->GetLocation();
		FVector lastPosition = it->GetLocation();
		++it;

		m_MaxDistanceFromFirstSample = 0.0f;
		m_TotalTraveledDistance = 0.0f;

		for (; it != m_History.end(); ++it)
		{
			FVector currentPosition = it->GetLocation();
			float distanceLast = (lastPosition - currentPosition).Size();
			m_TotalTraveledDistance += distanceLast;
			float distanceFirst = (firstPosition - currentPosition).Size();
			m_MaxDistanceFromFirstSample = FMath::Max(distanceFirst, m_MaxDistanceFromFirstSample);

			lastPosition = currentPosition;
		}
	}
}

bool FTrackerTransformHistory::HasCompleteHistory() const
{
	return m_History.Num() == m_TargetSampleCount;
}

float FTrackerTransformHistory::GetMaxDistanceFromFirstSample() const
{
	return m_MaxDistanceFromFirstSample;
}

float FTrackerTransformHistory::GetTotalDistanceTraveled() const
{
	return m_TotalTraveledDistance;
}

float FTrackerTransformHistory::GetAverageVelocity() const
{
	return m_TotalTraveledDistance / m_TargetSampleCount;
}

const FTransform& FTrackerTransformHistory::GetLatest() const
{
	return m_History.Last();
}
