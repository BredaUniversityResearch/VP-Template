#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"
#include "Containers/RingBuffer.h"

class FGetTrackerStaticTransformTask : public FTickableEditorObject
{

	static constexpr int MinStaticBaseStationOffsets = 4;
	static constexpr int SampleSizeSeconds = 5;
	static constexpr int SamplesPerSecond = 10;
	static constexpr float AverageVelocityThreshold = 0.5f;

public:
	explicit FGetTrackerStaticTransformTask(int32 a_TargetTrackerId);

	virtual void Tick(float DeltaTime) override;
	bool IsComplete() const;
	const FTransform& GetResult() const;
	//Returns the offsets of the base stations to the tracker. (tracker transform + offset = base station transform)
	TMap<int32, FTransform>& GetBaseStationResults();

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }
private:

	void TakeBaseStationSamples();
	bool HasCompleteBaseStationsHistoryAndBelowVelocityThreshold(float Threshold) const;
	void BuildStaticBaseStationResults();

	int32 TargetTrackerId;
	FTransform Result;
	//Store the offsets of the base station to the tracker. (tracker transform + offset = base station transform)
	TMap<int32, FTransform> BaseStationResults;	
	bool HasAcquiredTransform{ false };
	bool HasAcquiredBaseStationOffsets{ false };

	float SampleDeltaTimeAccumulator{ 0.0f };
	FTrackerTransformHistory TransformHistory;

	TMap<int32, FTrackerTransformHistory> BaseStationOffsets;
};

