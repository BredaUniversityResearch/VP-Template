// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

// =========================================================================
// Vicon Data Stream LiveLink Source implementation.
//
// Utilises a helper stream reader class which runs on a worker thread
// receiving data from Vicon and pushing through to the LiveLink client.
// =========================================================================
#pragma once

#include "ILiveLinkClient.h"
#include "ILiveLinkSource.h"
#include "LiveLinkViconDataStreamSourceSettings.h"
#include "ViconStream.h"
#include "ViconStreamFrameReader.h"

class ILiveLinkClient;

class LIVELINKDATASTREAM_API FLiveLinkViconDataStreamSource : public ILiveLinkSource
{
public:
  FLiveLinkViconDataStreamSource( const FText& InSourceType, const ViconStreamProperties& InViconStreamProps );
  virtual ~FLiveLinkViconDataStreamSource();

  // Begin ILiveLinkSource interface
  virtual void ReceiveClient( ILiveLinkClient* InClient, FGuid InSourceGuid ) override;

  virtual bool IsSourceStillValid() const override;

  virtual bool RequestSourceShutdown() override;

  virtual FText GetSourceType() const override;
  virtual FText GetSourceMachineName() const override;
  virtual FText GetSourceStatus() const override;

  virtual void InitializeSettings( ULiveLinkSourceSettings* Settings ) override;
  virtual TSubclassOf< ULiveLinkSourceSettings > GetSettingsClass() const override { return ULiveLinkDataStreamSourceSettings::StaticClass(); }
  virtual void OnSettingsChanged( ULiveLinkSourceSettings* Settings, const FPropertyChangedEvent& PropertyChangedEvent ) override;
  // End ILiveLinkSource interface

  bool IsConnected() const;

private:
  ILiveLinkClient* Client;

  // Our identifier in LiveLink
  FGuid SourceGuid;

  FText SourceType;

  ViconStreamProperties ViconStreamProps;
  FViconStreamFrameReader* ViconStreamFrameReader;
};
