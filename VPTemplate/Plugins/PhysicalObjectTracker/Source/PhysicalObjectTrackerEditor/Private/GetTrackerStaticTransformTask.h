#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"
#include "Containers/RingBuffer.h"

class FGetTrackerStaticTransformTask : public FTickableEditorObject
{
	static constexpr int SampleSizeSeconds = 5;
	static constexpr int SamplesPerSecond = 10;
	static constexpr float AverageVelocityThreshold = 0.5f;
public:
	explicit FGetTrackerStaticTransformTask(int32 a_TargetTrackerId);

	virtual void Tick(float DeltaTime) override;
	bool IsComplete() const;
	const FTransform& GetResult() const;

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }
private:
	int32 m_TargetTrackerId;
	FTransform m_Result;
	bool m_HasAcquiredTransform{ false };

	float SampleDeltaTimeAccumulator{ 0.0f };
	FTrackerTransformHistory m_TransformHistory;
};

