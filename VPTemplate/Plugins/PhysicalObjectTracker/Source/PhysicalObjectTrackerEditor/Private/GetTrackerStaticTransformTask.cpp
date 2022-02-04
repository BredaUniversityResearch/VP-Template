#include "GetTrackerStaticTransformTask.h"

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

	SampleDeltaTimeAccumulator += DeltaTime;
	constexpr float SecondsBetweenSamples = 1.0f / static_cast<float>(SamplesPerSecond);
	if (SampleDeltaTimeAccumulator > SecondsBetweenSamples)
	{
		SampleDeltaTimeAccumulator -= SecondsBetweenSamples;

		m_TransformHistory.TakeSample(m_TargetTrackerId);
		if (m_TransformHistory.HasCompleteHistory())
		{
			if (m_TransformHistory.GetAverageVelocity() < AverageVelocityThreshold)
			{
				m_HasAcquiredTransform = true;
				m_Result = m_TransformHistory.GetLatest();
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
