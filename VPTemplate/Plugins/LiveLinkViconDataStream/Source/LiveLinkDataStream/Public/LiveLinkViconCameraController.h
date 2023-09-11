// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "LiveLinkCameraController.h"

#include "LiveLinkViconCameraController.generated.h"

/**
 * LiveLink Controller for 
 */
UCLASS()
class LIVELINKDATASTREAM_API ULiveLinkViconCameraController : public ULiveLinkCameraController
{
  GENERATED_BODY()

public:
  ULiveLinkViconCameraController();

  //~ Begin ULiveLinkControllerBase interface
  virtual void Tick( float DeltaTime, const FLiveLinkSubjectFrameData& SubjectData ) override;
  //~ End ULiveLinkControllerBase interface
};
