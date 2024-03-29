#pragma once
#include "DetectTrackerShakeTask.h"
#include "GetTrackerStaticTransformTask.h"
#include "GetBaseStationTransformsTask.h"
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
		DetectingBaseStations,
		Done,
	};

public:
	FUpdateTrackerCalibrationAsset(UPhysicalObjectTrackingReferencePoint* a_TargetAsset);

	virtual void Tick(float DeltaTime) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }

	void OnCancelCalibration();
	void OnTrackerIdentified();
	void OnTrackerTransformAcquired(const FTransform& TrackerTransform, const TMap<int32, FTransform>& BaseStationOffsets);
	void OnBaseStationOffsetsAcquired(const TMap<int32, FTransform>& CalculatedBaseStationOffsets);
	void UpdateAsset() const;

	void Cleanup();
	bool IsCompleted() const;

	UPhysicalObjectTrackingReferencePoint* TargetAsset{ nullptr };
	ECalibrationState m_CalibrationState{ ECalibrationState::Initial };

	TSharedPtr<SNotificationItem> m_ProcessNotification;

	TUniquePtr<FDetectTrackerShakeTask> SelectControllerTask;
	TUniquePtr<FGetTrackerStaticTransformTask> GetTrackerStaticPositionTask;
	TUniquePtr<FGetBaseStationTransformsTask> GetBaseStationOffsetsTask;

	int32 TrackerId{ -1 };
	FTransform TrackerCalibrationTransform;
	TMap<int32, FTransform> CalibratedBaseStationOffsets;
	TSet<int32> StaticallyCalibratedBaseStations;

	const int32 MinBaseStationsCalibrated;
	const int32 MinBaseStationsCalibratedStatic;
};

