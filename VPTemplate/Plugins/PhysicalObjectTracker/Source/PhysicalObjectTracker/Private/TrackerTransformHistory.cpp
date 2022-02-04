#include "TrackerTransformHistory.h"

#include "PhysicalObjectTrackingFilterSettings.h"
#include "PhysicalObjectTrackingUtility.h"

#include "PhysicalObjectTracker.h"

FTrackerTransformHistory::FTrackerTransformHistory(int32 a_TargetSampleCount)
	: m_TargetSampleCount(a_TargetSampleCount)
	, m_History(a_TargetSampleCount)
{
}

void FTrackerTransformHistory::AddSample(const FTransform& a_Transform)
{
	m_History.Add(a_Transform);
	while (m_History.Num() > m_TargetSampleCount)
	{
		m_History.PopFrontValue();
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

void FTrackerTransformHistory::ClearSampleHistory()
{
	m_History.Reset();
}

void FTrackerTransformHistory::TakeSample(int32 a_TargetTrackerId)
{
	FVector trackerPosition;
	FQuat trackerRotation;
	if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(a_TargetTrackerId, trackerPosition, trackerRotation))
	{
		AddSample(FTransform(trackerRotation, trackerPosition));
		
	}
	else
	{
		ClearSampleHistory();
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

FTransform FTrackerTransformHistory::GetAveragedTransform(const UPhysicalObjectTrackingFilterSettings* FilterSettings) const
{
	if (FilterSettings == nullptr)
	{
		UE_LOG(LogPhysicalObjectTracker, Warning, TEXT("Tracker has null filter settings"));
		return GetLatest();
	}

	if (!HasCompleteHistory())
	{
		return GetLatest();
	}

	float curveLocation = FMath::Clamp((GetAverageVelocity() - FilterSettings->MinExpectedVelocity) / 
		(FilterSettings->MaxExpectedVelocity - FilterSettings->MinExpectedVelocity), 0.0f, 1.0f);
	float sampleCountUnit = FMath::Pow(1.0f - curveLocation, FilterSettings->FilterExponent);
	int32 sampleCount = FMath::RoundToInt(sampleCountUnit * m_TargetSampleCount);
	sampleCount = FMath::Clamp(sampleCount, 1, m_TargetSampleCount);

	GEngine->AddOnScreenDebugMessage(231231231, 0.0f, FColor::Emerald, FString::Printf(TEXT("Using %i samples"), sampleCount));

	return GetAveragedTransformOverSampleCount(sampleCount);
}

void FTrackerTransformHistory::SetFromFilterSettings(UPhysicalObjectTrackingFilterSettings* FilterSettings)
{
	if (FilterSettings != nullptr)
	{
		m_TargetSampleCount = FilterSettings->TargetSampleCount;
		m_History = TRingBuffer<FTransform>(m_TargetSampleCount);
		ClearSampleHistory();
	}
}

FTransform FTrackerTransformHistory::GetAveragedTransformOverSampleCount(int32 a_SampleCount) const
{
	FVector averageLocation = FVector::ZeroVector;
	FVector averageRotationEuler = FVector::ZeroVector;

	for (int i = 0; i < a_SampleCount; ++i)
	{
		const FTransform& sample = m_History[m_History.Num() - (i + 1)];
		averageLocation += sample.GetLocation();
		averageRotationEuler += sample.GetRotation().Euler();
	}
	averageLocation = averageLocation / a_SampleCount;
	averageRotationEuler = averageRotationEuler / a_SampleCount;

	return FTransform(FQuat::MakeFromEuler(averageRotationEuler), averageLocation);
}
