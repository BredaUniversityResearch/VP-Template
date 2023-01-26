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
		{FString{"LHB-4DA74639"}, FColor::Orange},
		{FString{"LHB-397A56CC"}, FColor::Blue},
		{FString{"LHB-1BEC1CA4"}, FColor::Cyan},
		{FString{"LHB-2239FAC8"}, FColor::Emerald},
		{FString{"LHB-2A1A0096"}, FColor::Turquoise},
		{FString{"LHB-B6A41014"}, FColor::Purple}
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
		const UPhysicalObjectTrackingReferencePoint* reference = targetComponent->GetTrackingReferencePoint();
		if (reference != nullptr)
		{
			const FTransform* worldReference = targetComponent->GetWorldReferencePoint();
			FPhysicalObjectTrackerEditor::DebugDrawTrackingReferenceLocations(reference, worldReference);

			TArray<int32> deviceIds;
			USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::TrackingReference, deviceIds);

			for (const int32 deviceId : deviceIds)
			{
				FVector position;
				FRotator rotation;
				if (USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(deviceId, position, rotation))
				{
					const FQuat baseStationRotationFix = FQuat(FVector(0.0f, 1.0f, 0.0f), FMath::DegreesToRadians(90.0f));

					FTransform transform = reference->ApplyTransformation(position, rotation.Quaternion() * baseStationRotationFix);
					if (worldReference != nullptr)
					{
						FTransform::Multiply(&transform, &transform, worldReference);
					}
					const FMatrix transformMatrix = transform.ToMatrixNoScale();

					FColor lightHouseColor = FColor::Black;

					FString lightHouseSerialId {};
					if(FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(deviceId, lightHouseSerialId))
					{
						//Determine the debug color of the light house.
						{
							const FColor* color = LightHouseColors.Find(lightHouseSerialId);
							if (color != nullptr)
							{
								lightHouseColor = *color;
							}
						}

						FTransform offsetTransform;
						if(reference->GetBaseStationWorldTransform(lightHouseSerialId, offsetTransform))
						{
							if (worldReference != nullptr)
							{
								FTransform::Multiply(&offsetTransform, &offsetTransform, worldReference);
							}
							const FMatrix offsetTransformMatrix = offsetTransform.ToMatrixNoScale();

							constexpr float scale = 0.7;
							const FColor offsetColor(lightHouseColor.R * scale, lightHouseColor.G * scale, lightHouseColor.B * scale);

							DrawWireBox(PDI, offsetTransformMatrix, FBox(FVector(-5.f), FVector(5.f)), offsetColor, 0, 1.5f);
							DrawDirectionalArrow(PDI, offsetTransformMatrix, offsetColor, 60.f, 5.f, 0, 1.5f);
							DrawWireFrustrum2(PDI, offsetTransformMatrix, LighthouseV2HorizontalFov, LighthouseV2HorizontalFov / LighthouseV2VerticalFov,
								LighthouseV2MinTrackingDistance, LighthouseV2MaxTrackingDistance, offsetColor, 0, 2.5f);
							DrawDirectionalArrow(PDI, offsetTransformMatrix, offsetColor, 150.f, 10.f, 0, 1.f);
						}
					}

					DrawWireBox(PDI, transformMatrix, FBox(FVector(-4.0f), FVector(4.0f)), lightHouseColor, 0, 2.0f);
					DrawWireFrustrum2(PDI, transformMatrix, LighthouseV2HorizontalFov, LighthouseV2HorizontalFov / LighthouseV2VerticalFov,
						LighthouseV2MinTrackingDistance, LighthouseV2MaxTrackingDistance, lightHouseColor, 0, 2.0f);
					DrawDirectionalArrow(PDI, transformMatrix, lightHouseColor, 100.f, 10.f, 0, 2.f);

					const FColor rawColor(lightHouseColor.R * 0.75, lightHouseColor.G * 0.75, lightHouseColor.B * 0.75);
					FMatrix rawTransform = FTransform(rotation, position).ToMatrixNoScale();
					DrawWireBox(PDI, rawTransform, FBox(FVector(-7.f), FVector(7.f)), rawColor, 0, 0.5f);
					DrawDirectionalArrow(PDI, rawTransform, rawColor, 100.f, 10.f, 0, 1.f);

					const FColor worldColor(lightHouseColor.R * 0.5, lightHouseColor.G * 0.5, lightHouseColor.B * 0.5);
					FTransform trackingToWorld = GEngine->XRSystem->GetTrackingToWorldTransform();
					rawTransform = (trackingToWorld * FTransform(rotation, position)).ToMatrixNoScale();
					DrawWireBox(PDI, rawTransform, FBox(FVector(-5.f), FVector(5.f)), worldColor, 0, 0.5f);
					DrawDirectionalArrow(PDI, rawTransform, worldColor, 100.f, 10.f, 0, 1.f);

					const FColor originColor(lightHouseColor.R * 0.25, lightHouseColor.G * 0.25, lightHouseColor.B * 0.25);
					FTransform trackingOrigin = FTransform::Identity;
					GEngine->XRSystem->GetTrackingOriginTransform(EHMDTrackingOrigin::Floor, trackingOrigin);
					rawTransform = (trackingOrigin.Inverse() * FTransform(rotation, position)).ToMatrixNoScale();
					DrawWireBox(PDI, rawTransform, FBox(FVector(-3.f), FVector(3.f)), originColor, 0, 0.5f);
					DrawDirectionalArrow(PDI, rawTransform, originColor, 100.f, 10.f, 0, 1.f);

					FQuat poseRotation;
					FVector poseLocation;
					if(GEngine->XRSystem->GetCurrentPose(deviceId, poseRotation, poseLocation))
					{
						rawTransform = FTransform(poseRotation, poseLocation).ToMatrixNoScale();
						DrawWireBox(PDI, rawTransform, FBox(FVector(-2.f), FVector(2.f)), FColor::Red, 0, 0.5f);
						DrawDirectionalArrow(PDI, rawTransform, originColor, 100.f, 10.f, 0, 1.f);
					}
					
				}
			}
		}
	}
}
