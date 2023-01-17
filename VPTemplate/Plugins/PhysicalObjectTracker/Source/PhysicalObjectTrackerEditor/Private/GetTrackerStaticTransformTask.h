#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"
#include "Containers/RingBuffer.h"

class FGetTrackerStaticTransformTask : public FTickableEditorObject
{

	static constexpr int MinStaticBaseStationOffsets = 2;
	static constexpr int SampleSizeSeconds = 5;
	static constexpr int SamplesPerSecond = 10;
	static constexpr float AverageVelocityThreshold = 0.5f;

public:
	explicit FGetTrackerStaticTransformTask(int32 a_TargetTrackerId);

	virtual void Tick(float DeltaTime) override;
	bool IsComplete() const;
	const FTransform& GetResult() const;
	//Returns the offsets of the base stations to the tracker. (tracker transform + offset = base station transform)
	const TMap<int32, FTransform>& GetBaseStationResults() const;

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }
private:

	void TakeBaseStationSamples(const FTransform& TrackerTransform);
	bool HasCompleteBaseStationsHistory();
	void BuildStaticBaseStationResults();

	int32 m_TargetTrackerId;
	FTransform m_Result;
	//Store the offsets of the base station to the tracker. (tracker transform + offset = base station transform)
	TMap<int32, FTransform> m_BaseStationResults;	
	bool m_HasAcquiredTransform{ false };

	float SampleDeltaTimeAccumulator{ 0.0f };
	FTrackerTransformHistory m_TransformHistory;

	TMap<int32, FTrackerTransformHistory> BaseStationOffsets;
};

