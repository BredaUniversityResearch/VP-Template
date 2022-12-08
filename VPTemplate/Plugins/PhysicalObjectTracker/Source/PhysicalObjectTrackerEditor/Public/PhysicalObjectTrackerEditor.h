#pragma once

#include "CoreMinimal.h"

class FComponentVisualizer;
class FPhysicalObjectTrackingReferenceCalibrationHandler;
class FDetectTrackerShakeTask;
class UPhysicalObjectTrackingReferencePoint;

DECLARE_DELEGATE_OneParam(FShakeTaskFinished, uint32)

class FPhysicalObjectTrackerEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void DebugDrawTrackingReferenceLocations(const UPhysicalObjectTrackingReferencePoint* PhysicalReferencePoint, const FTransform* WorldTransform);

private:
	void OnDeviceDetectionStarted(class UPhysicalObjectTrackingComponent* TargetTrackingComponent);

	void StopDeviceSelection();

	TUniquePtr<FPhysicalObjectTrackingReferenceCalibrationHandler> m_TrackingCalibrationHandler{};

	TUniquePtr<FDetectTrackerShakeTask> m_ShakeDetectTask;
	TSharedPtr<SNotificationItem> m_ShakeProcessNotification;

	TSharedPtr<FComponentVisualizer> m_ComponentVisualizer;

};
