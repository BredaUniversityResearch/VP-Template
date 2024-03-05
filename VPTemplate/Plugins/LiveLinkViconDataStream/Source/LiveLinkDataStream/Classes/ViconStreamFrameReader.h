// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

// =========================================================================
// Vicon Data Stream Frame Reader implementation.
//
// Connects to a Vicon data stream via a ViconStream helper class.
// Runs on a worker thread and receives skeleton data and bone
// transformations for each frame of animation.
// Pushes skeleton and frame data to the LiveLink client.
// =========================================================================

#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include <ViconStream.h>
#include <DataStreamClient.h>

class FLiveLinkViconDataStreamSource;


class LIVELINKDATASTREAM_API ViconStreamProperties
{
public:
  static ViconStreamProperties FromString( const FString& i_rPropsString );
  const FString ToString() const;

  FText m_ServerName;
  FText m_SubjectFilter;
  uint32 m_PortNumber;

  bool m_bRetimed;
  float m_RetimeOffset;
  bool m_bLightweight;
  bool m_bUsePrefetch;
  bool m_bScaled;

  bool m_bLogOutput;
};

class FViconStreamFrameReader : public FRunnable
{
public:
  FViconStreamFrameReader( ILiveLinkClient* i_pClient, const ViconStreamProperties& i_rViconStreamProps, const FGuid& i_rSourceGuid );
  virtual ~FViconStreamFrameReader();

  static const std::string UNLABELED_MARKER;
  static const std::string LABELED_MARKER;

  // Begin FRunnable interface.
  virtual bool Init();
  virtual uint32 Run();
  virtual void Stop();
  // End FRunnable interface

  void Connect();

  void Shutdown();

  bool IsConnected() const;

  FString ConstructServerAddress();

  void SetLightweightEnabled( bool i_bLightweight );
  void SetMarkerEnabled( bool i_bStreamMarker );
  void SetUnlabeledMarkerEnabled( bool i_bStreamMarker );
  void ShowAllVideoCamera( bool i_bShow );

private:
  // Cached representation of static data for transform / animation subjects
  class FCachedSubject
  {
  public:
    TArray<std::string> Markers;
    TArray<std::string> Bones;
  };

  void HandleSubjectData();
  void HandleCameraData();
  void HandleMarkerData();
  bool AddSubjectStaticDataToLiveLink( const FString& i_rSubjectName, TArray< std::string >& o_rSubjectBones, TArray<std::string>& o_rMarkerNames );
  void ClearCamerasFromLiveLink( const TSet< FString >& i_rStaleCameras );
  void ClearMarkerFromLiveLink( const FLiveLinkSubjectKey& i_rMarkerKey );

  // Handle markers not attached to subjects
  void HandleMarkerData(bool bLabeled);

  // Utility to get a list of property names of the form {index}_{axis}
  TArray<FName> GetGenericMarkerPropertyNames(unsigned int& MarkerCount);

private:
  void ConnectInternal();

  // Adds _X, _Y, and _Z to the marker names, returning a list whose length is three times the input
  TArray<FName> MarkerPropertiesFromNames(const TArray<std::string>& i_rMarkerNames);

  ILiveLinkClient* m_pLiveLinkClient;
  ViconStreamProperties m_ViconStreamProps;
  FGuid m_SourceGuid;

  int32 m_LastFrameNumber;
  FThreadSafeBool m_bStopTask;
  FRunnableThread* m_pThread;
  FCriticalSection* m_pMutex;

  ViconStream m_DataStream;

  TMap< FString, FCachedSubject> m_CachedSubjects;
  TSet< FString > m_CachedCameras;
  TMap< FString, unsigned int> m_CachedMarkerCounts;
  bool m_bLightweight;
  bool m_bMarker;
  bool m_bUnlabeledMarker;
  bool m_bShowAllVideoCamera;
  TArray< FString > m_SubjectAllowed;

};
