// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "LiveLinkViconDataStreamSource.h"
#include "ILiveLinkDataStreamModule.h"
#include "LiveLinkViconDataStreamSourceSettings.h"

FLiveLinkViconDataStreamSource::FLiveLinkViconDataStreamSource( const FText& InSourceType, const ViconStreamProperties& InViconStreamProps )
: SourceType( InSourceType )
, ViconStreamProps( InViconStreamProps )
, ViconStreamFrameReader( nullptr )
{
}

FLiveLinkViconDataStreamSource::~FLiveLinkViconDataStreamSource()
{
  if( ViconStreamFrameReader != nullptr )
  {
    delete ViconStreamFrameReader;
    ViconStreamFrameReader = nullptr;
  }
}

void FLiveLinkViconDataStreamSource::ReceiveClient( ILiveLinkClient* InClient, FGuid InSourceGuid )
{
  Client = InClient;
  SourceGuid = InSourceGuid;
  ViconStreamFrameReader = new FViconStreamFrameReader( InClient, ViconStreamProps, SourceGuid );
}

bool FLiveLinkViconDataStreamSource::IsSourceStillValid() const
{
  return ViconStreamFrameReader != NULL;
}

bool FLiveLinkViconDataStreamSource::RequestSourceShutdown()
{
  ViconStreamFrameReader->Shutdown();
  return true;
}

FText FLiveLinkViconDataStreamSource::GetSourceType() const
{
  return SourceType;
}

FText FLiveLinkViconDataStreamSource::GetSourceMachineName() const
{
  if( ViconStreamFrameReader )
  {
    return FText::FromString( ViconStreamFrameReader->ConstructServerAddress() );
  }
  else
  {
    return FText();
  }
}

FText FLiveLinkViconDataStreamSource::GetSourceStatus() const
{
  if( !IsConnected() )
  {
    return FText::FromString( "Not Connected" );
  }
  else
  {
    return FText::FromString( "Connected" );
  }
}

bool FLiveLinkViconDataStreamSource::IsConnected() const
{
  if( ViconStreamFrameReader == nullptr )
  {
    return false;
  }
  else
  {
    return ViconStreamFrameReader->IsConnected();
  }
}

void FLiveLinkViconDataStreamSource::InitializeSettings( ULiveLinkSourceSettings* Settings )
{
  ULiveLinkDataStreamSourceSettings* DataStreamSettings = Cast< ULiveLinkDataStreamSourceSettings >( Settings );
  ViconStreamFrameReader->SetLightweightEnabled( DataStreamSettings->EnableLightweight );
  ViconStreamFrameReader->SetMarkerEnabled( DataStreamSettings->StreamMarkerData );
  ViconStreamFrameReader->SetUnlabeledMarkerEnabled( DataStreamSettings->StreamUnlabeledMarkerData );
  ViconStreamFrameReader->ShowAllVideoCamera( DataStreamSettings->ShowAllVideoCamera );
}

void FLiveLinkViconDataStreamSource::OnSettingsChanged( ULiveLinkSourceSettings* Settings, const FPropertyChangedEvent& PropertyChangedEvent )
{
  ULiveLinkDataStreamSourceSettings* DataStreamSettings = Cast< ULiveLinkDataStreamSourceSettings >( Settings );

  ViconStreamFrameReader->SetLightweightEnabled( DataStreamSettings->EnableLightweight );
  ViconStreamFrameReader->SetMarkerEnabled( DataStreamSettings->StreamMarkerData );
  ViconStreamFrameReader->SetUnlabeledMarkerEnabled( DataStreamSettings->StreamUnlabeledMarkerData );
  ViconStreamFrameReader->ShowAllVideoCamera( DataStreamSettings->ShowAllVideoCamera );
  ILiveLinkSource::OnSettingsChanged( Settings, PropertyChangedEvent );
}
