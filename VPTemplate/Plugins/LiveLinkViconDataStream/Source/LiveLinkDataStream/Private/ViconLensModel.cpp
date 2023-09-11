// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "ViconLensModel.h"

const FName UViconLensModel::LensModelName( TEXT( "ViconLensModel" ) );

UScriptStruct* UViconLensModel::GetParameterStruct() const
{
  return FViconDistortionParameters::StaticStruct();
}

FName UViconLensModel::GetModelName() const
{
  return LensModelName;
}

FName UViconLensModel::GetShortModelName() const
{
  return TEXT("Vicon");
}
