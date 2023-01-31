#pragma once

#include "CoreMinimal.h"

class FComponentVisualizer;
class FPhysicalObjectTrackingReferenceCalibrationHandler;
class FPhysicalObjectTrackerSerialIdSelectionHandler;
class UPhysicalObjectTrackingReferencePoint;

class FPhysicalObjectTrackerEditor : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void DebugDrawTrackingReferenceLocations(const UPhysicalObjectTrackingReferencePoint* PhysicalReferencePoint, const FTransform* WorldTransform);

private:

	TUniquePtr<FPhysicalObjectTrackingReferenceCalibrationHandler> m_TrackingCalibrationHandler{};
	TUniquePtr<FPhysicalObjectTrackerSerialIdSelectionHandler> m_TrackerSerialIdSelectionHandler{};

	TSharedPtr<FComponentVisualizer> m_ComponentVisualizer;

};
