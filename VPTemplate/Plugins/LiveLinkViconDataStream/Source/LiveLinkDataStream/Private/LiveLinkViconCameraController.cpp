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
  UActorComponent* AttachedCameraComponent = GetAttachedComponent();
 
  if( UCineCameraComponent* CineCameraComponent = Cast< UCineCameraComponent >( AttachedCameraComponent ) )
  {
    const FLiveLinkLensFrameData* FrameData = SubjectData.FrameData.Cast< FLiveLinkLensFrameData >();

    if( FrameData )
    {
      float NewFilmbackRatio = CineCameraComponent->Filmback.SensorWidth / CineCameraComponent->Filmback.SensorHeight;
      float FrameDataRatio = FrameData->FxFy.Y / FrameData->FxFy.X;
      // If user input of filmback setting changed, we check if it matches what we expect. 
      if( !FMath::IsNearlyEqual( CurrentFilmbackRatio, NewFilmbackRatio ) )
      {
        // We expect the user input of filmback ratio to be the same as our calibration streamed from LiveLink.
        if( FMath::IsNearlyEqual( NewFilmbackRatio, FrameDataRatio ) )
        {
          float FocalLength = FrameData->FxFy.X * CineCameraComponent->Filmback.SensorWidth;
          CineCameraComponent->SetCurrentFocalLength( FocalLength );
          UE_LOG( LogTemp, Log, TEXT("Focal length updated to %.4f" ), FocalLength );
        }
        // If it doesn't match, we notify what is expected in the log and do not update anything.
        else
        {
          float Height = CineCameraComponent->Filmback.SensorWidth / FrameDataRatio;
          UE_LOG( LogTemp, Warning, TEXT( "Expecting sensor height %.4f, focal length not updated" ), Height );
        }
        // Keep track of the input filmback ratio
        CurrentFilmbackRatio = NewFilmbackRatio;
      }
    }
  }
  ULiveLinkCameraController::Tick( DeltaTime, SubjectData );
}
