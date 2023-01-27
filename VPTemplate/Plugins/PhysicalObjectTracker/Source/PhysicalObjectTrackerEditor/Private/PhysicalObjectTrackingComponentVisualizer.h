#pragma once
#include "ComponentVisualizer.h"

class FPhysicalObjectTrackingComponentVisualizer: public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

	void DrawBaseStationReference(FPrimitiveDrawInterface* PDI, FColor Color, FTransform BaseStationTransform) const;
};

