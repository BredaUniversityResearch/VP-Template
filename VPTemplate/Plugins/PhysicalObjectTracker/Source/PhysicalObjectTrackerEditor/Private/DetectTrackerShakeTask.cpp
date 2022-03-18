#include "DetectTrackerShakeTask.h"

#include "SteamVRFunctionLibrary.h"

FTrackerTransform::FTrackerTransform(FVector a_Position)
	: Position(a_Position)
{
}

void FDetectTrackerShakeTask::Tick(float DeltaTime)
{
	if (SelectedController != -1)
	{
		return;
	}

	m_DeltaTimeAccumulator += DeltaTime;
	constexpr float UpdateDelay = 1.0f / static_cast<float>(SampleCountPerSecond);
	if (m_DeltaTimeAccumulator > UpdateDelay)
	{
		m_DeltaTimeAccumulator -= UpdateDelay;
		if (DeltaTime > UpdateDelay)
		{
			m_DeltaTimeAccumulator = 0.0f;
		}
		ensure(!IsInRenderingThread());
		TArray<int32> controllerIds;
		USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::Controller, controllerIds);
		TArray<int32> otherIds;
		USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::Other, otherIds);
		controllerIds.Append(MoveTemp(otherIds));

		if (controllerIds.Num() == 0)
		{
			m_IsComplete = true;
			OnTaskFinished.ExecuteIfBound(-1);
			return;
		}

		for (int32 controllerId : controllerIds)
		{
			FTrackerTransformHistory& trackerHistory = m_TrackerHistory.FindOrAdd(controllerId, FTrackerTransformHistory(SampleCountPerSecond * SampleSizeSeconds));
			trackerHistory.TakeSample(controllerId);

			if (trackerHistory.HasCompleteHistory())
			{
				if (trackerHistory.GetMaxDistanceFromFirstSample() * 2.0f < trackerHistory.GetTotalDistanceTraveled() && trackerHistory.GetTotalDistanceTraveled() > 60.0f)
				{
					SelectedController = controllerId;
					OnTaskFinished.ExecuteIfBound(SelectedController);
					OnTaskFinished.Unbind();
					m_IsComplete = true;
					return;
				}
			}
		}
	}
}

ETickableTickType FDetectTrackerShakeTask::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool FDetectTrackerShakeTask::IsTickable() const
{
	return !m_IsComplete;
}

bool FDetectTrackerShakeTask::IsComplete() const
{
	return m_IsComplete;
}

bool FDetectTrackerShakeTask::IsFailed() const
{
	return m_IsComplete && SelectedController == -1;
}

FText FDetectTrackerShakeTask::GetFailureReason() const
{
	return NSLOCTEXT("FPhysicalObjectTrackerEditor", "NoTrackersFound", "No trackers or controllers found to check for calibration.");
}
