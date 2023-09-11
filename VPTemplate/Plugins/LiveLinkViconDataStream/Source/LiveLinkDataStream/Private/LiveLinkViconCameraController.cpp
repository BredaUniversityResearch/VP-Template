// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "LiveLinkViconCameraController.h"

#include "CineCameraComponent.h"
#include "LiveLinkLensRole.h"
#include "LiveLinkLensTypes.h"

ULiveLinkViconCameraController::ULiveLinkViconCameraController()
: ULiveLinkCameraController()
{
}

void ULiveLinkViconCameraController::Tick( float DeltaTime, const FLiveLinkSubjectFrameData& SubjectData )
{
  UActorComponent* AttachedCameraComponent =  GetAttachedComponent();
 
  if( UCineCameraComponent* CineCameraComponent = Cast< UCineCameraComponent >( AttachedCameraComponent ) )
  {
    const FLiveLinkLensFrameData* FrameData = SubjectData.FrameData.Cast< FLiveLinkLensFrameData >();

    if( FrameData )
    {
      float FilmbackRatio = CineCameraComponent->Filmback.SensorWidth / CineCameraComponent->Filmback.SensorHeight;
      float FrameDataRatio = FrameData->FxFy.Y / FrameData->FxFy.X;
      if( FMath::IsNearlyEqual( FilmbackRatio, FrameDataRatio, 1e-6f ) )
      {
        CineCameraComponent->SetCurrentFocalLength( FrameData->FxFy.X * CineCameraComponent->Filmback.SensorWidth );
      }
      else
      {
        UE_LOG( LogTemp, Warning, TEXT( "Expecting image width height ratio of %.6f but current ratio is %.6f, no change to focal length" ), FrameDataRatio, FilmbackRatio );
      }
    }
  }
  ULiveLinkCameraController::Tick( DeltaTime, SubjectData );
}
