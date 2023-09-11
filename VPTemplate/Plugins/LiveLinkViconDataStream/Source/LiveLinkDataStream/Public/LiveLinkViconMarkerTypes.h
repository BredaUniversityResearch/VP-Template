// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "LiveLinkTypes.h"
#include "LiveLinkViconMarkerTypes.generated.h"

/**
 * Static data for ViconMarker data. 
 */
USTRUCT( BlueprintType )
struct LIVELINKDATASTREAM_API FLiveLinkViconMarkerStaticData : public FLiveLinkBaseStaticData
{
  GENERATED_BODY()

  // Whether location in frame data should be used
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "LiveLink" )
  bool bIsLabeled = true;
};

/**
 * Dynamic data for Transform 
 */
USTRUCT( BlueprintType )
struct LIVELINKDATASTREAM_API FLiveLinkViconMarkerFrameData : public FLiveLinkBaseFrameData
{
  GENERATED_BODY()

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Properties", Interp )
  TArray< FVector > m_MarkerData;
};

/**
 * Facility structure to handle marker data in blueprint
 */
USTRUCT( BlueprintType )
struct LIVELINKDATASTREAM_API FLiveLinkViconMarkerBlueprintData : public FLiveLinkBaseBlueprintData
{
  GENERATED_BODY()

  // Static data that should not change every frame
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Vicon Marker Data" )
  FLiveLinkViconMarkerStaticData StaticData;

  // Dynamic data that can change every frame
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Vicon Marker Data" )
  FLiveLinkViconMarkerFrameData FrameData;
};
