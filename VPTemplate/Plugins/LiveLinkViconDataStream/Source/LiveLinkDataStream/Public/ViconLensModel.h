// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "CameraCalibrationCore/Public/Models/LensModel.h"
#include "ViconLensModel.generated.h"

USTRUCT( BlueprintType )
struct LIVELINKDATASTREAM_API FViconDistortionParameters
{
  GENERATED_BODY()

public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Distortion" )
  float K1 = 0.0f;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Distortion" )
  float K2 = 0.0f;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Distortion" )
  float K3 = 0.0f;
};

UCLASS( BlueprintType, meta = ( DisplayName = "ViconLensModel" ) )
class LIVELINKDATASTREAM_API UViconLensModel : public ULensModel
{
  GENERATED_BODY()

public:
  //~ Begin ULensModel interface
  virtual UScriptStruct* GetParameterStruct() const override;
  virtual FName GetModelName() const override;
  virtual FName GetShortModelName() const override;
  //~ End ULensModel interface

  static const FName LensModelName;
};
