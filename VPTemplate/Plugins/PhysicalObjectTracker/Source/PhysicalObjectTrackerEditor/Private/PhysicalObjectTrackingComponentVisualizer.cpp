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

	void DrawAxisBox(FPrimitiveDrawInterface* PDI, const FColor& Color, const FTransform& Transformation)
	{
		const FMatrix transformationMatrix = Transformation.ToMatrixNoScale();
		const FMatrix transformationMatrixUp = (FTransform(FQuat(FVector::RightVector, FMath::DegreesToRadians(-90.f))) * Transformation).ToMatrixNoScale();
		const FMatrix transformationMatrixRight = (FTransform(FQuat(FVector::UpVector, FMath::DegreesToRadians(90.f))) * Transformation).ToMatrixNoScale();

		DrawWireBox(PDI, transformationMatrix, FBox(FVector(-5.0f), FVector(5.0f)), Color, 0, 2.0f);

		DrawDirectionalArrow(PDI, transformationMatrix, FColor::Red, 50.f, 10.f, 0, 1.f);				//X-axis	
		DrawDirectionalArrow(PDI, transformationMatrixUp, FColor::Blue, 50.f, 10.f, 0, 1.f);			//Z-axis
		DrawDirectionalArrow(PDI, transformationMatrixRight, FColor::Green, 50.f, 10.f, 0, 1.f);		//Y-axis
	}

	void DrawBaseStationReference(FPrimitiveDrawInterface* PDI, const FColor& Color, const FTransform& Transformation, float LineThickness = 2.f)
	{
		DrawAxisBox(PDI, Color, Transformation);
		DrawWireFrustrum2(PDI, Transformation.ToMatrixNoScale(), 
			LighthouseV2HorizontalFov, 
			LighthouseV2HorizontalFov / LighthouseV2VerticalFov, 
			LighthouseV2MinTrackingDistance, 
			LighthouseV2MaxTrackingDistance, 
			Color, 0, LineThickness);
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
					FTransform transform = reference->ApplyTransformation(position, rotation.Quaternion());
					if (worldReference != nullptr)
					{
						FTransform::Multiply(&transform, &transform, worldReference);
					}

					FColor lightHouseColor = FColor::Black;

					FString lightHouseSerialId {};
					if(FPhysicalObjectTrackingUtility::FindSerialIdFromDeviceId(deviceId, lightHouseSerialId))
					{
						//Determine the debug color of the light house.
						{
							const FColor* color = BaseStationColors.Find(lightHouseSerialId);
							if (color != nullptr)
							{
								lightHouseColor = *color;
							}
						}

						//Visualize the base stations with the stored calibration data.
						FTransform offsetTransform;
						if(reference->GetBaseStationWorldTransform(lightHouseSerialId, offsetTransform))
						{
							if (worldReference != nullptr)
							{
								FTransform::Multiply(&offsetTransform, &offsetTransform, worldReference);
							}

							constexpr float colorRatio = 0.7;
							const FColor offsetColor(lightHouseColor.R * colorRatio, lightHouseColor.G * colorRatio, lightHouseColor.B * colorRatio);
							DrawBaseStationReference(PDI, offsetColor, offsetTransform, 3.f);
						}
					}

					//Visualize the base stations with the current SteamVR data
					DrawBaseStationReference(PDI, lightHouseColor, transform);

				}
			}

			//Tracker visualization.

			TArray<int32> trackerIds;
			USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType::Other, trackerIds);

			for(const int32 trackerId : trackerIds)
			{
				FVector trackerPosition;
				FQuat trackerRotation;
				FTransform trackerTransform = FTransform::Identity;
				if(FPhysicalObjectTrackingUtility::GetTrackedDevicePositionAndRotation(trackerId, trackerPosition, trackerRotation))
				{
					const FColor trackerColor = FColor::Orange;
					trackerTransform = FPhysicalObjectTrackingUtility::FixTrackerTransform(FTransform(trackerRotation, trackerPosition));

					/*FTransform trackerWorldTransform = reference->ApplyTransformation(trackerTransform.GetLocation(), trackerTransform.GetRotation());
					if(worldReference != nullptr)
					{
						FTransform::Multiply(&trackerWorldTransform, &trackerWorldTransform, worldReference);
					}
					DrawAxisBox(PDI, trackerColor, trackerWorldTransform);*/

					FTransform trackerWorldTransformRelative = reference->GetTrackerWorldTransform(trackerTransform);
					if (worldReference != nullptr)
					{
						FTransform::Multiply(&trackerWorldTransformRelative, &trackerWorldTransformRelative, worldReference);
					}
					DrawAxisBox(PDI, trackerColor, trackerWorldTransformRelative);

				}
			}
		}
	}
}