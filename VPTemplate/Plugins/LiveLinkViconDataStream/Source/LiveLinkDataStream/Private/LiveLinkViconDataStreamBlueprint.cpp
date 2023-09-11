// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

// Blueprint function library for useful functions to be exposed to blueprints
// Sam Goodwin 2019

#include "LiveLinkViconDataStreamBlueprint.h"
#include "Features/IModularFeature.h"
#include "Features/IModularFeatures.h"
#include "Kismet/KismetStringLibrary.h"
#include "LiveLinkViconDataStreamSource.h"
#include "Misc/Timecode.h"

void ULiveLinkViconDataStreamBlueprint::CreateViconLiveLinkSource( FString ServerName, int32 PortNumber, FString SubjectFilter, bool bIsRetimed, bool bUsePreFetch, bool bIsScaled, bool bLogOutput, float Offset, FLiveLinkSourceHandle& SourceHandle )
{
  IModularFeatures& ModularFeatures = IModularFeatures::Get();

  TSharedPtr< FLiveLinkViconDataStreamSource > NewSource = nullptr;

  if( ModularFeatures.IsModularFeatureAvailable( ILiveLinkClient::ModularFeatureName ) )
  {
    ILiveLinkClient* LiveLinkClient = &ModularFeatures.GetModularFeature< ILiveLinkClient >( ILiveLinkClient::ModularFeatureName );

    ViconStreamProperties props;
    props.m_ServerName = FText::FromString( ServerName );
    props.m_SubjectFilter = FText::FromString( SubjectFilter );
    props.m_PortNumber = PortNumber;
    props.m_bScaled = bIsScaled;
    props.m_bRetimed = bIsRetimed;
    props.m_RetimeOffset = Offset;
    props.m_bLogOutput = bLogOutput;
    props.m_bUsePrefetch = bUsePreFetch;

    NewSource = MakeShareable( new FLiveLinkViconDataStreamSource( FText::FromString( "Vicon Live Link" ), props ) );

    LiveLinkClient->AddSource( NewSource );
    SourceHandle.SetSourcePointer( NewSource );
  }
  else
  {
    SourceHandle.SetSourcePointer( nullptr );
  }
}

FTimecode ULiveLinkViconDataStreamBlueprint::TimecodeFromFrameNumber( const FFrameNumber& FrameNumber, const FFrameRate& FrameRate )
{
  return FTimecode::FromFrameNumber( FrameNumber, FrameRate);
}
