// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "ViconLensDistortionModelHandler.h"

#include "CameraCalibrationSettings.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/NumericLimits.h"
//#include <algorithm>

void UViconLensDistortionModelHandler::InitializeHandler()
{
  // Material is stored in plugins/LiveLinkViconDataStream/Content/Materials
  // Made based on the default Spherical Material
  // The Material might need updating with Engine version update.
  //
  LensModelClass = UViconLensModel::StaticClass();
  DistortionMaterial = FSoftObjectPath( TEXT( "/LiveLinkViconDataStream/Materials/ViconDistortionPostProcess.ViconDistortionPostProcess" ) );
  DistortionDisplacementMaterial = FSoftObjectPath( TEXT( "/LiveLinkViconDataStream/Materials/ViconDistortionDisplacementMap.ViconDistortionDisplacementMap" ) );
}

FVector2D UViconLensDistortionModelHandler::ComputeDistortedUV( const FVector2D& InUndistortedUV ) const
{
  // ComputeDistortedUV is used for calculating overscan factor.
  // This needs to be the same as the material expression in
  // Material asset ViconDistortionDisplacementMap which is
  // used to compute displacement UV
  //
  // These distances cannot be zero in real-life. If they are, the current distortion state must be bad
  if( ( CurrentState.FocalLengthInfo.FxFy.X == 0.0f ) || ( CurrentState.FocalLengthInfo.FxFy.Y == 0.0f ) )
  {
    return InUndistortedUV;
  }

  // aspects:
  // image-dimension-normalised - divided by image dimensions - induces non-uniformity (non-square pixels)
  // offset by principal point
  // multiplied by focal length

  // input is:
  // image-dimension-normalised (divided by image dimensions - so non-uniform (non-square pixels))
  // times by focal length
  // not offset by PP
  // output is the same, but distorted

  // to apply Vicon calibration, need:
  // no image-normalisation
  // by default Vicon calibration is on data AFTER multiplication by focal length, but we can transform it to operate on data pre multiplication
  // By:
  // K1 = K1_orig * F^2
  // K2 = K2_orig * F^4
  // K3 = K3_orig * F^6
  //
  // NormalizedDistanceFromImageCenter is offset by PP, and scaled back into world dimensions (not multiplied by Focal length)

  auto NormalizedDistanceFromImageCenter = ( InUndistortedUV - CurrentState.ImageCenter.PrincipalPoint ) / CurrentState.FocalLengthInfo.FxFy;

  const FVector2D DistanceSquared = NormalizedDistanceFromImageCenter * NormalizedDistanceFromImageCenter;
  const float RSquared = DistanceSquared.X + DistanceSquared.Y;
  const float RadialDistortion = 1.0f + ( ViconParameters.K1 * RSquared ) + ( ViconParameters.K2 * RSquared * RSquared ) + ( ViconParameters.K3 * RSquared * RSquared * RSquared );
  auto CorrectedPosCentred = RadialDistortion * NormalizedDistanceFromImageCenter;
  auto DistortedUV = CurrentState.FocalLengthInfo.FxFy * CorrectedPosCentred + FVector2D( 0.5, 0.5 );
  return DistortedUV;
}

void UViconLensDistortionModelHandler::InitDistortionMaterials()
{
  if( DistortionPostProcessMID == nullptr )
  {
    UMaterialInterface* MaterialParent = DistortionMaterial.LoadSynchronous();
    DistortionPostProcessMID = UMaterialInstanceDynamic::Create( MaterialParent, this );
  }

  if( DistortionDisplacementMapMID == nullptr )
  {
    UMaterialInterface* MaterialParent = DistortionDisplacementMaterial.LoadSynchronous();
    DistortionDisplacementMapMID = UMaterialInstanceDynamic::Create( MaterialParent, this );
  }

  DistortionPostProcessMID->SetTextureParameterValue( "DistortionDisplacementMap", DistortionDisplacementMapRT );

  SetDistortionState( CurrentState );
}

void UViconLensDistortionModelHandler::UpdateMaterialParameters()
{
  //Helper function to set material parameters of an MID
  const auto SetDistortionMaterialParameters = [this]( UMaterialInstanceDynamic* const MID ) {
    MID->SetScalarParameterValue( "k1", ViconParameters.K1 );
    MID->SetScalarParameterValue( "k2", ViconParameters.K2 );
    MID->SetScalarParameterValue( "k3", ViconParameters.K3 );

    MID->SetScalarParameterValue( "cx", CurrentState.ImageCenter.PrincipalPoint.X );
    MID->SetScalarParameterValue( "cy", CurrentState.ImageCenter.PrincipalPoint.Y );

    MID->SetScalarParameterValue( "fx", CurrentState.FocalLengthInfo.FxFy.X );
    MID->SetScalarParameterValue( "fy", CurrentState.FocalLengthInfo.FxFy.Y );
  };

  SetDistortionMaterialParameters( DistortionDisplacementMapMID );
}

void UViconLensDistortionModelHandler::InterpretDistortionParameters()
{
  LensModelClass->GetDefaultObject< ULensModel >()->FromArray< FViconDistortionParameters >( CurrentState.DistortionInfo.Parameters, ViconParameters );
}
