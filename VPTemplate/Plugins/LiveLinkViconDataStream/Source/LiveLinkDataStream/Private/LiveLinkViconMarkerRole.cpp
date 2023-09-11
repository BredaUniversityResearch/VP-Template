// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "LiveLinkViconMarkerRole.h"
#include "LiveLinkViconMarkerTypes.h"

#define LOCTEXT_NAMESPACE "LiveLinkRole"

UScriptStruct* ULiveLinkViconMarkerRole::GetStaticDataStruct() const
{
  return FLiveLinkViconMarkerStaticData::StaticStruct();
}

UScriptStruct* ULiveLinkViconMarkerRole::GetFrameDataStruct() const
{
  return FLiveLinkViconMarkerFrameData::StaticStruct();
}

UScriptStruct* ULiveLinkViconMarkerRole::GetBlueprintDataStruct() const
{
  return FLiveLinkViconMarkerBlueprintData::StaticStruct();
}

bool ULiveLinkViconMarkerRole::InitializeBlueprintData( const FLiveLinkSubjectFrameData& InSourceData, FLiveLinkBlueprintDataStruct& OutBlueprintData ) const
{
  bool bSuccess = false;

  FLiveLinkViconMarkerBlueprintData* BlueprintData = OutBlueprintData.Cast< FLiveLinkViconMarkerBlueprintData >();
  const FLiveLinkViconMarkerStaticData* StaticData = InSourceData.StaticData.Cast< FLiveLinkViconMarkerStaticData >();
  const FLiveLinkViconMarkerFrameData* FrameData = InSourceData.FrameData.Cast< FLiveLinkViconMarkerFrameData >();
  if( BlueprintData && StaticData && FrameData )
  {
    GetStaticDataStruct()->CopyScriptStruct( &BlueprintData->StaticData, StaticData );
    GetFrameDataStruct()->CopyScriptStruct( &BlueprintData->FrameData, FrameData );
    bSuccess = true;
  }

  return bSuccess;
}

FText ULiveLinkViconMarkerRole::GetDisplayName() const
{
  return LOCTEXT( "ViconMarkerRole", "ViconMarker" );
}

#undef LOCTEXT_NAMESPACE
