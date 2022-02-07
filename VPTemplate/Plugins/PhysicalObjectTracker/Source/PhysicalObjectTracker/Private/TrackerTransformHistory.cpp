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
		FTransform lastTransform = *it;
		++it;

		m_MaxDistanceFromFirstSample = 0.0f;
		m_TotalTraveledDistance = 0.0f;
		m_TotalRotatedDegrees = 0.0f;

		for (; it != m_History.end(); ++it)
		{
			FTransform currentTransform = *it;
			float distanceLast = (lastTransform.GetLocation() - currentTransform.GetLocation()).Size();
			m_TotalTraveledDistance += distanceLast;
			float distanceFirst = (firstPosition - currentTransform.GetLocation()).Size();
			m_MaxDistanceFromFirstSample = FMath::Max(distanceFirst, m_MaxDistanceFromFirstSample);
			FQuat deltaRotation = lastTransform.GetRotation() * currentTransform.GetRotation().Inverse();
			m_TotalRotatedDegrees += deltaRotation.Euler().Size();

			lastTransform = currentTransform;
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

float FTrackerTransformHistory::GetTotalRotatedDegrees() const
{
	return m_TotalRotatedDegrees;
}

float FTrackerTransformHistory::GetAverageVelocity() const
{
	return m_TotalTraveledDistance / m_TargetSampleCount;
}

float FTrackerTransformHistory::GetAverageRotationalVelocity() const
{
	return m_TotalRotatedDegrees / m_TargetSampleCount;
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

	float linearVelocityPercentage = FMath::Clamp((GetAverageVelocity() - FilterSettings->MinExpectedLinearVelocity) / 
		(FilterSettings->MaxExpectedLinearVelocity - FilterSettings->MinExpectedLinearVelocity), 0.0f, 1.0f);
	float rotationVelocityPercentage = FMath::Clamp((GetAverageRotationalVelocity() - FilterSettings->MinExpectedRotationalVelocity) /
		(FilterSettings->MaxExpectedRotationalVelocity - FilterSettings->MinExpectedRotationalVelocity), 0.0f, 1.0f);
	float curveLocation = FMath::Max(linearVelocityPercentage, rotationVelocityPercentage);
	float sampleCountUnit = FMath::Pow(1.0f - curveLocation, FilterSettings->FilterExponent);
	int32 sampleCount = FMath::RoundToInt(sampleCountUnit * m_TargetSampleCount);
	sampleCount = FMath::Clamp(sampleCount, 1, m_TargetSampleCount);

	GEngine->AddOnScreenDebugMessage(231231231, 0.0f, FColor::Emerald, FString::Printf(TEXT("Using %i samples Loc: %f / (%f-%f) Rot: %f (%f-%f)"), sampleCount, 
		GetAverageVelocity(), FilterSettings->MinExpectedLinearVelocity, FilterSettings->MaxExpectedLinearVelocity, 
		GetAverageRotationalVelocity(), FilterSettings->MinExpectedRotationalVelocity, FilterSettings->MaxExpectedRotationalVelocity));

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
