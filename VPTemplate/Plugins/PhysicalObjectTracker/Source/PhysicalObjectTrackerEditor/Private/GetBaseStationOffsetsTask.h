#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"

class FGetBaseStationOffsetsTask : public FTickableEditorObject
{

	static constexpr int SampleSizeSeconds = 5;
	static constexpr int SamplesPerSecond = 10;


public:
	explicit FGetBaseStationOffsetsTask(
		int32 TargetTracker, 
		int32 TargetNumBaseStationOffsets, 
		const TMap<int32, FTransform>* CompletedBaseStations);

	virtual void Tick(float DeltaTime) override;

	bool IsComplete() const;
	const TMap<int32, FTransform>& GetResults() const;


private:

	void TakeBaseStationSamples();
	bool HasCompleteBaseStationsHistory();
	void BuildBaseStationResults();

	float SampleDeltaTimeAccumulator{ 0.0f };
	bool HasAcquiredTransforms{ false };
	TMap<int32, FTransform> BaseStationResults;

	TMap<int32, FTrackerTransformHistory> BaseStationOffsets;
	const int32 TargetTrackerId;
	const int32 TargetNumBaseStationOffsets;
	const TMap<int32, FTransform>* CompletedBaseStationsIds;

};