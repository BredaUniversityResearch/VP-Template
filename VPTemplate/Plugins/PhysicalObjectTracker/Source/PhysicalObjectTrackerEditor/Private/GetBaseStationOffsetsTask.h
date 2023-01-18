#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"

class FGetBaseStationOffsetsTask : public FTickableEditorObject
{

	static constexpr int SampleSizeSeconds = 5;
	static constexpr int SamplesPerSecond = 10;


public:
	explicit FGetBaseStationOffsetsTask(
		int32 InTargetTrackerId, 
		int32 InTargetNumBaseStationOffsets,
		const FTransform& InTargetTrackerNeutralOffsetToSteamVROrigin,
		TMap<int32, FTransform>* InCalibratedBaseStationOffsets);

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
	//The offset between the SteamVR origin and Led Volume Origin (0,0,0) when the target tracker is at the Led Volume Origin (neutral position).
	const FTransform TargetTrackerNeutralOffsetToSteamVROrigin;
	TMap<int32, FTransform>* CalibratedBaseStationOffsets;

};