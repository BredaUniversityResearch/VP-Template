#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"

class FGetBaseStationTransformsTask : public FTickableEditorObject
{

	static constexpr int SampleSizeSeconds = 5;
	static constexpr int SamplesPerSecond = 10;


public:
	FGetBaseStationTransformsTask(
		int32 InTargetNumBaseStationTransforms,
		const TMap<int32, FTransform>* InCalibratedBaseStationTransforms);

	virtual void Tick(float DeltaTime) override;

	bool IsComplete() const;
	TMap<int32, FTransform>& GetResults();

	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }
private:

	void TakeBaseStationSamples();
	bool HasCompleteBaseStationsHistory();
	void BuildBaseStationResults();

	float SampleDeltaTimeAccumulator{ 0.0f };
	bool HasAcquiredTransforms{ false };

	TMap<int32, FTrackerTransformHistory> BaseStationTransforms;
	TMap<int32, FTransform> BaseStationResults;
	
	const int32 TargetNumBaseStationTransforms;
	
	const TMap<int32, FTransform>* CalibratedBaseStationTransforms;

};