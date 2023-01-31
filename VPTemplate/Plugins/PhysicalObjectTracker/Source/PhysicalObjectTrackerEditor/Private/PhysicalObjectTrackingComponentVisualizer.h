#pragma once
#include "ComponentVisualizer.h"

class FPhysicalObjectTrackingComponentVisualizer: public FComponentVisualizer
{
public:

	inline static const TMap<FString, FColor> BaseStationColors
	{
		{FString{"LHB-4DA74639"}, FColor::Red},		//Right front
		{FString{"LHB-397A56CC"}, FColor::Green},	//Right back
		{FString{"LHB-1BEC1CA4"}, FColor::Blue},	//Middle Right
		{FString{"LHB-2239FAC8"}, FColor::Yellow},
		{FString{"LHB-B6A41014"}, FColor::Magenta}, //Left back
		{FString{"LHB-2A1A0096"}, FColor::Cyan},
	};

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};

