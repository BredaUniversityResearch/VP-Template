// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "Algo/Transform.h"
#include "Containers/StringConv.h"
#include "Containers/UnrealString.h"
#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"
#include "Logging/LogMacros.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkCameraTypes.h"

DECLARE_LOG_CATEGORY_CLASS( LogViconStream, Display, All )

#ifdef CPP
#pragma push_macro( "CPP" )
#undef CPP
#define RESTORE_POINT_CPP
#endif

#include <DataStreamClient.h>
#include <DataStreamRetimingClient.h>
#include <IDataStreamClientBase.h>

//// With the new move semantics behaviour of the LiveLink API,
//// we may wish to rethink the use of this class and rely on
//// querying the client for data we have added rather
//// than cacheing it to eliminate copies
//class ViconStreamSubject
//{
//public:
//  // New API
//  FLiveLinkSubjectKey m_SubjectKey;
//  FLiveLinkStaticDataStruct m_RefSkeleton;
//  FLiveLinkFrameDataStruct m_LastPose;
//
//  FQualifiedFrameTime m_LastTC;
//};

struct FLiveLinkLensStaticData;
struct FLiveLinkLensFrameData;
enum EResult
{
  ESuccess,
  EError,
};

enum EStreamMode
{
  EPush,
  EPullPreFetch,
  EPull
};

// A Wrapper class converting data from Vicon
// to Unreal
class ViconStream
{
public:
  ViconStream();
  ~ViconStream();

  EResult Connect( const FString& i_rServer, bool i_bRetimed, bool i_bLogOutput );
  EResult Reconnect();
  bool IsConnected() const;
  void Disconnect();
  unsigned int GetFrameNumber();

  EResult SetLightWeightEnabled( bool i_bEnabled );
  void SetMarkerDataEnabled( bool i_bEnabled );
  void SetUnlabeledMarkerDataEnabled( bool i_bEnabled );
  void UseKalman( bool i_bEnabled );
  EResult GetFrame();
  EResult SetOffset( float Offset );
  void SetUseScaling( bool i_bUseScaling );
  EResult SetSubjectFilter( const FString& i_rSubjectFilter );
  EResult SetStreamMode( const EStreamMode& i_rMode );

  void SetAxisMappingOculus();
  void SetAxisMappingDefault();

  EResult GetSubjectCount( int& o_rCount ) const;
  EResult GetSubjectName( int Index, FString& o_rName ) const;

  EResult GetSegmentCountForSubject( const std::string& i_rSubjectNme, unsigned int& o_rCount ) const;
  EResult GetSegmentNameForSubject( const std::string& i_rSubjectNme, int Index, FString& o_rSegName ) const;
  EResult GetSegmentParentNameForSubject( const std::string& i_rSubjectName, const std::string& i_rSegName, FString& o_rSegName ) const;

  EResult GetSegmentLocalPose( const std::string& i_rSubjectName, const std::string& i_rSegmentName, FTransform& o_rPose );

  bool GetPoseForSubject( const std::string& InName, const TArray< std::string >& BoneNames, FLiveLinkFrameDataStruct& OutSubject );
  EResult GetSubjectNames( TArray< FString >& SubjectNames );

  EResult GetRootPose( const std::string& i_rSubjectName, FVector& o_rPosition, FQuat& o_rOrientation );

  EResult GetDynamicCameraCount( int& o_rCount ) const;

  // change to set to be able to compare
  EResult GetDynamicCameraNames( TSet< FString >& o_rNameList ) const;

  EResult GetVideoCameraNames( TSet< FString >& o_rNameList ) const;

  EResult GetCameraTransformFrameData( const std::string& i_rCameraName, FLiveLinkTransformFrameData& OutSubject );
  EResult GetLensStaticData( const std::string& i_rCameraName, FLiveLinkLensStaticData& LensStaticData );
  EResult GetLensFrameData( const std::string& i_rCameraName, FLiveLinkLensFrameData& LensFrameData );

  EResult GetLabelledMarkers( TArray< FVector >& o_rMarkerList );
  EResult GetUnlabelledMarkers( TArray< FVector >& o_rMarkerList );

  bool IsRetimed() { return m_bRetimed; }

  bool m_bUseViconHMD;
  bool m_LogDebug;
  bool m_bUseScaling;

private:
  EResult GetSegmentScale( const std::string& i_rSubjectName, const std::string& i_rSegmentName, FVector& o_rScale );
  bool IsViconServerYup();

  FString m_ServerIP;
  float m_Offset;

  ViconDataStreamSDK::CPP::IDataStreamClientBase* m_pClient;
  bool m_bRetimed;
  bool m_bLogOutput;

  ViconDataStreamSDK::CPP::Client m_Client;
  ViconDataStreamSDK::CPP::RetimingClient m_RetimingClient;

  std::map< std::pair< std::string, std::string >, FTransform > m_CachedSubject;
};

#ifdef RESTORE_POINT_CPP
#pragma pop_macro( "CPP" )
#undef RESTORE_POINT_CPP
#endif
