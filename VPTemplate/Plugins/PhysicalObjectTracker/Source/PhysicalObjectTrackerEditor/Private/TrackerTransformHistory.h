#pragma once
#include "Containers/RingBuffer.h"

class FTrackerTransformHistory
{
public:
	FTrackerTransformHistory(int32 a_SampleCount, int32 a_TrackerId);

	void TakeSample();

	bool HasCompleteHistory() const;
	float GetMaxDistanceFromFirstSample() const;
	float GetTotalDistanceTraveled() const;
	float GetAverageVelocity() const;
	const FTransform& GetLatest() const;

private:
	const int32 m_TargetSampleCount;
	const int32 m_TargetTrackerId;
	TRingBuffer<FTransform> m_History;

	float m_MaxDistanceFromFirstSample{ 0.0f };
	float m_TotalTraveledDistance{ 0.0f };
};

