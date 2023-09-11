// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "LensDistortionModelHandlerBase.h"

#include "ViconLensModel.h"
#include "ViconLensDistortionModelHandler.generated.h"

/** Lens distortion handler for a vicon lens model */
UCLASS( BlueprintType )
class LIVELINKDATASTREAM_API UViconLensDistortionModelHandler : public ULensDistortionModelHandlerBase
{
  GENERATED_BODY()

protected:
  //~ Begin ULensDistortionModelHandlerBase interface
  virtual void InitializeHandler() override;
  virtual FVector2D ComputeDistortedUV( const FVector2D& InScreenUV ) const override;
  virtual void InitDistortionMaterials() override;
  virtual void UpdateMaterialParameters() override;
  virtual void InterpretDistortionParameters() override;
  //~ End ULensDistortionModelHandlerBase interface

private:
  /** Vicon lens distortion parameters (k1, k2, k3 ) */
  FViconDistortionParameters ViconParameters;
  TSoftObjectPtr< UMaterialInterface > DistortionMaterial;
  TSoftObjectPtr< UMaterialInterface > DistortionDisplacementMaterial;
  TSoftObjectPtr< UMaterialInterface > UndistortionDisplacementMaterial;
};
