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
	explicit FGetTrackerStaticTransformTask(int32 a_TargetTrackerId, int32 MinStaticBaseStationOffsets);

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

	const int32 TargetTrackerId {-1};
	const int32 MinBaseStationResults;

	FTransform TrackerResult;
	TMap<int32, FTransform> BaseStationResults; //Store the offsets of the base station to the tracker. (tracker transform + offset = base station transform)

	bool HasAcquiredTransformsAndOffsets{ false };
	float SampleDeltaTimeAccumulator{ 0.0f };

	FTrackerTransformHistory TrackerTransforms;
	TMap<int32, FTrackerTransformHistory> BaseStationTransforms;
};

