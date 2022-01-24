#pragma once
#include "DetectTrackerShakeTask.h"
#include "GetTrackerStaticTransformTask.h"
#include "TickableEditorObject.h"
#include "Widgets/Notifications/SNotificationList.h"

class UPhysicalObjectTrackingReferencePoint;

class FUpdateTrackerCalibrationAsset : public FTickableEditorObject
{
	enum class ECalibrationState
	{
		Initial,
		SelectingController,
		WaitingForStaticPosition,
		Done,
	};
public:
	FUpdateTrackerCalibrationAsset(UPhysicalObjectTrackingReferencePoint* a_TargetAsset);

	virtual void Tick(float DeltaTime) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }

	void OnCancelCalibration();
	void OnTrackerIdentified(int32 TrackerId);
	void OnTrackerTransformAcquired(const FTransform& Transform);

	void Cleanup();
	bool IsCompleted() const;

	UPhysicalObjectTrackingReferencePoint* TargetAsset{ nullptr };
	ECalibrationState m_CalibrationState{ ECalibrationState::Initial };

	TSharedPtr<SNotificationItem> m_ProcessNotification;

	TUniquePtr<FDetectTrackerShakeTask> SelectControllerTask;
	TUniquePtr<FGetTrackerStaticTransformTask> GetTrackerStaticPositionTask;
};

