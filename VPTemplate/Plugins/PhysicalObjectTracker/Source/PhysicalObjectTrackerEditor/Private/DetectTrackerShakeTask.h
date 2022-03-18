#pragma once
#include "TickableEditorObject.h"
#include "TrackerTransformHistory.h"
#include "Containers/RingBuffer.h"
#include "PhysicalObjectTrackerEditor.h"

struct FTrackerTransform
{
public:
	FTrackerTransform(FVector a_Position);

	const FVector Position;
};
class FDetectTrackerShakeTask : public FTickableEditorObject
{
	static constexpr int SampleCountPerSecond = 20;
	static constexpr int SampleSizeSeconds = 1;
public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }

	bool IsComplete() const;
	bool IsFailed() const;
	FText GetFailureReason() const;

	int32 SelectedController = -1;
	FShakeTaskFinished OnTaskFinished;
private:
	bool m_IsComplete = false;
	TMap<int32, FTrackerTransformHistory> m_TrackerHistory{};
	float m_DeltaTimeAccumulator{ 0.0f };

};

