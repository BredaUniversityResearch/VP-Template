#pragma once
#include "Containers/RingBuffer.h"

class UPhysicalObjectTrackingFilterSettings;

class PHYSICALOBJECTTRACKER_API FTrackerTransformHistory
{
public:
	FTrackerTransformHistory() = default;
	FTrackerTransformHistory(int32 a_TargetSampleCount);

	void AddSample(const FTransform& a_Transform);
	void ClearSampleHistory();
	void TakeSample(int32 a_TargetTrackerId);

	bool HasCompleteHistory() const;
	float GetMaxDistanceFromFirstSample() const;
	float GetTotalDistanceTraveled() const;
	float GetTotalRotatedDegrees() const;
	float GetAverageVelocity() const;
	float GetAverageRotationalVelocity() const;
	const FTransform& GetLatest() const;
	FTransform GetAveragedTransform(const UPhysicalObjectTrackingFilterSettings* FilterSettings) const;

	void SetFromFilterSettings(UPhysicalObjectTrackingFilterSettings* FilterSettings);

private:
	FTransform GetAveragedTransformOverSampleCount(int32 a_SampleCount) const;

	int32 m_TargetSampleCount{ 10 };
	TRingBuffer<FTransform> m_History{ m_TargetSampleCount }; //Last is most recent.

	float m_MaxDistanceFromFirstSample{ 0.0f };
	float m_TotalTraveledDistance{ 0.0f };
	float m_TotalRotatedDegrees{ 0.0f };
};

