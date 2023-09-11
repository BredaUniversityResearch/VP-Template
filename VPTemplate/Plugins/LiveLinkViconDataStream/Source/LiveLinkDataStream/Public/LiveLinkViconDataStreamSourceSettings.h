// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkSourceSettings.h"
#include "LiveLinkViconDataStreamSourceSettings.generated.h"

UCLASS()
class LIVELINKDATASTREAM_API ULiveLinkDataStreamSourceSettings : public ULiveLinkSourceSettings
{
  GENERATED_BODY()

public:
  ULiveLinkDataStreamSourceSettings()
  {
    EnableLightweight = true;
    StreamMarkerData = false;
    StreamUnlabeledMarkerData = false;
    ShowAllVideoCamera = false;
  };
  UPROPERTY( EditAnywhere, Category = DataStreamSettings )
  bool EnableLightweight;

  UPROPERTY( EditAnywhere, Category = DataStreamSettings, AdvancedDisplay, meta = ( EditCondition = "!EnableLightweight" ) )
  bool StreamMarkerData;

  UPROPERTY( EditAnywhere, Category = DataStreamSettings, AdvancedDisplay, meta = ( EditCondition = "!EnableLightweight" ) )
  bool StreamUnlabeledMarkerData;

  UPROPERTY( EditAnywhere, Category = DataStreamSettings, AdvancedDisplay )
  bool ShowAllVideoCamera;
};
