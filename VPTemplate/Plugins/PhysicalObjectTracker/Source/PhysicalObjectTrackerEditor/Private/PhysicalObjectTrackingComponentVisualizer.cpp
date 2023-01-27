#include "PhysicalObjectTrackingComponentVisualizer.h"

#include "IXRTrackingSystem.h"
#include "PhysicalObjectTrackingComponent.h"
#include "PhysicalObjectTrackingReferencePoint.h"
#include "SteamVRFunctionLibrary.h"
#include "PhysicalObjectTrackerEditor.h"
#include "PhysicalObjectTrackingUtility.h"

namespace
{
	static constexpr float LighthouseV2HorizontalFov = 160.0f;
	static constexpr float LighthouseV2VerticalFov = 115.0f;
	static constexpr float LighthouseV2MinTrackingDistance = 10.0f;
	static constexpr float LighthouseV2MaxTrackingDistance = 700.0f;

	static const TMap<FString, FColor> LightHouseColors
	{
		{FString{"LHB-4DA74639"}, FColor::Red},		//Right front
		{FString{"LHB-397A56CC"}, FColor::Green},	//Right back
		{FString{"LHB-1BEC1CA4"}, FColor::Blue},	//Middle Right
		{FString{"LHB-2239FAC8"}, FColor::Yellow},
		{FString{"LHB-B6A41014"}, FColor::Magenta}, //Left back
		{FString{"LHB-2A1A0096"}, FColor::Cyan},
	};

	void DrawWireFrustrum(FPrimitiveDrawInterface* PDI, const FMatrix& FrustumToWorld, const FColor& Color, uint8 DepthPriorityGroup, float Thickness, float DepthBias = 0.0f, bool ScreenSpace = false)
	{
		FVector Vertices[2][2][2];
		for (uint32 Z = 0; Z < 2; Z++)
		{
			for (uint32 Y = 0; Y < 2; Y++)
			{
				for (uint32 X = 0; X < 2; X++)
				{
					FVector4 UnprojectedVertex = FrustumToWorld.TransformFVector4(
						FVector4(
							(X ? -1.0f : 1.0f),
							(Y ? -1.0f : 1.0f),
							(Z ? 0.0f : 1.0f),
							1.0f
						)
					);
					Vertices[X][Y][Z] = FVector(UnprojectedVertex) / UnprojectedVertex.W;
				}
			}
		}

		PDI->DrawLine(Vertices[0][0][0], Vertices[0][0][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[1][0][0], Vertices[1][0][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[0][1][0], Vertices[0][1][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[1][1][0], Vertices[1][1][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);

		PDI->DrawLine(Vertices[0][0][0], Vertices[0][1][0], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[1][0][0], Vertices[1][1][0], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[0][0][1], Vertices[0][1][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[1][0][1], Vertices[1][1][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);

		PDI->DrawLine(Vertices[0][0][0], Vertices[1][0][0], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[0][1][0], Vertices[1][1][0], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[0][0][1], Vertices[1][0][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Vertices[0][1][1], Vertices[1][1][1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
	}

	void DrawWireFrustrum2(FPrimitiveDrawInterface* PDI, const FMatrix& Transform, float HorizontalFOV, float Aspect, float StartDistance, float EndDistance, const FColor& Color, uint8 DepthPriorityGroup, float Thickness, float DepthBias = 0.0f, bool ScreenSpace = false)
	{
		//Shamelessly stolen from the UDrawFrustumComponent
		const FVector Direction(1, 0, 0);
		const FVector LeftVector(0, 1, 0);
		const FVector UpVector(0, 0, 1);

		FVector Verts[8];

		// FOVAngle controls the horizontal angle.
		const float HozHalfAngleInRadians = FMath::DegreesToRadians(HorizontalFOV * 0.5f);

		float HozLength = StartDistance * FMath::Tan(HozHalfAngleInRadians);
		float VertLength = HozLength / Aspect;

		// near plane verts
		Verts[0] = (Direction * StartDistance) + (UpVector * VertLength) + (LeftVector * HozLength);
		Verts[1] = (Direction * StartDistance) + (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[2] = (Direction * StartDistance) - (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[3] = (Direction * StartDistance) - (UpVector * VertLength) + (LeftVector * HozLength);

		HozLength = EndDistance * FMath::Tan(HozHalfAngleInRadians);
		VertLength = HozLength / Aspect;

		// far plane verts
		Verts[4] = (Direction * EndDistance) + (UpVector * VertLength) + (LeftVector * HozLength);
		Verts[5] = (Direction * EndDistance) + (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[6] = (Direction * EndDistance) - (UpVector * VertLength) - (LeftVector * HozLength);
		Verts[7] = (Direction * EndDistance) - (UpVector * VertLength) + (LeftVector * HozLength);

		for (int32 X = 0; X < 8; ++X)
		{
			Verts[X] = Transform.TransformPosition(Verts[X]);
		}

		PDI->DrawLine(Verts[0], Verts[1], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[1], Verts[2], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[2], Verts[3], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[3], Verts[0], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);

		PDI->DrawLine(Verts[4], Verts[5], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[5], Verts[6], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[6], Verts[7], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[7], Verts[4], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);

		PDI->DrawLine(Verts[0], Verts[4], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[1], Verts[5], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[2], Verts[6], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
		PDI->DrawLine(Verts[3], Verts[7], Color, DepthPriorityGroup, Thickness, DepthBias, ScreenSpace);
	}
}

void FPhysicalObjectTrackingComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FComponentVisualizer::DrawVisualization(Component, View, PDI);

	const UPhysicalObjectTrackingComponent* targetComponent = Cast<UPhysicalObjectTrackingComponent>(Component);
	if (targetComponent != nullptr)
	{
		FVector trackerPos = FVector::ZeroVector;
		FQuat trackerRot = FQuat::Identity;
		if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(targetComponent->CurrentTargetDeviceId, trackerPos, trackerRot))
		{
			GEngine->AddOnScreenDebugMessage(1231231245234ull, 0.0f, FColor::White, FString::Printf(TEXT("Tracker: Pos: [%.2f %.2f %.2f] Rot: [%.2f %.2f %.2f %.2f]"),
				trackerPos.X, trackerPos.Y, trackerPos.Z, trackerRot.X, trackerRot.Y, trackerRot.Z, trackerRot.W));
		}

		const UPhysicalObjectTrackingReferencePoint* reference = targetComponent->GetTrackingReferencePoint();
		if (reference != nullptr)
		{
			const FTransform* worldReference = targetComponent->GetWorldReferencePoint();
			FPhysicalObjectTrackerEditor::DebugDrawTrackingReferenceLocations(reference, worldReference);

			TArray<int32> deviceIds;
			USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::TrackingReference, deviceIds);

			for (const int32 deviceId : deviceIds)
			{
				FVector baseStationPosition;
				FQuat baseStationRotation;
				if (FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(deviceId, baseStationPosition, baseStationRotation))
				{
					FColor lightHouseColor = FColor::Black;
					FString lightHouseSerialId{};
					if (FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(deviceId, lightHouseSerialId))
					{
						//Determine the debug color of the light house.
						{
							const FColor* color = LightHouseColors.Find(lightHouseSerialId);
							if (color != nullptr)
							{
								lightHouseColor = *color;
							}
						}
					}



					//SteamVR space output.
					FColor rawColor(lightHouseColor.R * 0.75, lightHouseColor.G * 0.75, lightHouseColor.B * 0.75);
					FTransform baseStationTransform = FTransform(baseStationRotation, baseStationPosition);
					DrawBaseStationReference(PDI, rawColor, baseStationTransform);

					FTransform baseStationOffset = baseStationTransform * FTransform(trackerRot, trackerPos).Inverse();
					rawColor = FColor(lightHouseColor.R, lightHouseColor.G, lightHouseColor.B);
					DrawBaseStationReference(PDI, rawColor, baseStationOffset);

				}
			}
		}
	}
}

void FPhysicalObjectTrackingComponentVisualizer::DrawBaseStationReference(FPrimitiveDrawInterface* PDI, const FColor Color, FTransform BaseStationTransform) const
{
	DrawWireBox(PDI, BaseStationTransform.ToMatrixNoScale(), FBox(FVector(-7.f), FVector(7.f)), Color, 0, 0.5f);
	DrawDirectionalArrow(PDI, BaseStationTransform.ToMatrixNoScale(), Color, 100.f, 10.f, 0, 1.f);
	DrawDirectionalArrow(PDI, (FTransform(FQuat::MakeFromEuler(FVector(0, 90, 0)), FVector::ZeroVector) * BaseStationTransform).ToMatrixNoScale(), Color, 25.f, 10.f, 0, 1.f);
}
