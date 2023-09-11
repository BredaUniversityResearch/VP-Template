// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.


#include "ViconStreamFrameReader.h"
#include "ILiveLinkDataStreamModule.h"

#include "Async/Async.h"
#include "LiveLinkLensRole.h"
#include "LiveLinkLensTypes.h"
#include "LiveLinkTypes.h"
#include "LiveLinkViconMarkerRole.h"
#include "LiveLinkViconMarkerTypes.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"

ViconStreamProperties ViconStreamProperties::FromString( const FString& i_rPropsString )
{
  ViconStreamProperties Props;
  FString ServerName;
  // Parse properties from input string. If parsing fails, we provide a default value
  if( !FParse::Value( *i_rPropsString, TEXT( "ServerName=" ), ServerName ) )
  {
    Props.m_ServerName = FText::FromString( "localhost" );
  }
  Props.m_ServerName = FText::FromString( ServerName );

  FString SubjectFilter;
  if( !FParse::Value( *i_rPropsString, TEXT( "SubjectFilter=" ), SubjectFilter ) )
  {
    Props.m_SubjectFilter = FText::FromString( "" );
  }
  Props.m_SubjectFilter = FText::FromString( SubjectFilter );

  FString portnumber;
  if( !FParse::Value( *i_rPropsString, TEXT( "PortNumber=" ), portnumber ) )
  {
    Props.m_PortNumber = 801;
  }
  Props.m_PortNumber = FCString::Atoi( *portnumber );
  if( !FParse::Value( *i_rPropsString, TEXT( "RetimeOffset=" ), Props.m_RetimeOffset ) )
  {
    Props.m_RetimeOffset = 0;
  }

  // FParse doesn't handle bool
  FString Retimed;
  if( FParse::Value( *i_rPropsString, TEXT( "Retimed=" ), Retimed ) )
  {
    Retimed == "True" ? Props.m_bRetimed = true : Props.m_bRetimed = false;
  }
  else
  {
    Props.m_bRetimed = false;
  }

  FString LogOutput;
  if( FParse::Value( *i_rPropsString, TEXT( "LogOutput=" ), LogOutput ) )
  {
    LogOutput == "True" ? Props.m_bLogOutput = true : Props.m_bLogOutput = false;
  }
  else
  {
    Props.m_bLogOutput = false;
  }

  FString UsePrefetch;
  if( FParse::Value( *i_rPropsString, TEXT( "UsePretch=" ), UsePrefetch ) )
  {
    UsePrefetch == "True" ? Props.m_bUsePrefetch = true : Props.m_bUsePrefetch = false;
  }
  else
  {
    Props.m_bUsePrefetch = true;
  }

  FString Scaled;
  if( FParse::Value( *i_rPropsString, TEXT( "Scaled=" ), Scaled ) )
  {
    Scaled == "True" ? Props.m_bScaled = true : Props.m_bScaled = false;
  }
  else
  {
    Props.m_bScaled = true;
  }

  return Props;
}

const FString ViconStreamProperties::ToString() const
{
  FString PropertiesString = FString::Printf( TEXT( "ServerName=\"%s\"" ), *m_ServerName.ToString() );
  PropertiesString.Append( FString::Printf( TEXT( "SubjectFilter=\"%s\"" ), *m_SubjectFilter.ToString() ) );
  PropertiesString.Append( FString::Printf( TEXT( "PortNumber=\"%d\"" ), m_PortNumber ) );
  PropertiesString.Append( FString::Printf( TEXT( "Retimed=\"%s\"" ), m_bRetimed ? TEXT( "True" ) : TEXT( "False" ) ) );
  PropertiesString.Append( FString::Printf( TEXT( "RetimeOffset=\"%d\"" ), m_RetimeOffset ) );
  PropertiesString.Append( FString::Printf( TEXT( "LogOutput=\"%s\"" ), m_bLogOutput ? TEXT( "True" ) : TEXT( "False" ) ) );
  PropertiesString.Append( FString::Printf( TEXT( "UsePrefetch=\"%s\"" ), m_bUsePrefetch ? TEXT( "True" ) : TEXT( "False" ) ) );
  PropertiesString.Append( FString::Printf( TEXT( "Scaled=\"%s\"" ), m_bScaled ? TEXT( "True" ) : TEXT( "False" ) ) );

  return PropertiesString;
}

FViconStreamFrameReader::FViconStreamFrameReader( ILiveLinkClient* i_pClient, const ViconStreamProperties& i_rViconStreamProps, const FGuid& i_rSourceGuid )
: m_pLiveLinkClient( i_pClient )
, m_ViconStreamProps( i_rViconStreamProps )
, m_SourceGuid( i_rSourceGuid )
, m_LastFrameNumber( -1 )
, m_bStopTask( false )
, m_pThread( nullptr )
{
  Connect();
}

FViconStreamFrameReader::~FViconStreamFrameReader()
{
  Shutdown();

  {
    m_pThread->WaitForCompletion();
    delete m_pThread;
    m_pThread = nullptr;
  }
}

bool FViconStreamFrameReader::Init()
{
  return true;
}

uint32 FViconStreamFrameReader::Run()
{
  ConnectInternal();

  // If we don't connect, quit the thread immediately
  if( !m_DataStream.IsConnected() )
  {
    return 0;
  }

  while( !m_bStopTask )
  {
    EResult r = m_DataStream.GetFrame();
    if( r != ESuccess )
    {
      FPlatformProcess::Sleep( 0.001f );
      continue;
    }

    auto ViconFrameNumber = m_DataStream.GetFrameNumber();

    bool bRetimed = m_DataStream.IsRetimed();
    if( bRetimed )
    {
      HandleSubjectData();
      continue;
    }
    // no new frame
    if( ViconFrameNumber == m_LastFrameNumber )
    {
      FPlatformProcess::Sleep( 0.001f );
      continue;
    }
    else
    {
      m_LastFrameNumber = ViconFrameNumber;
      HandleSubjectData();
      HandleCameraData();
      HandleMarkerData();
    }
  }

  m_CachedSubjectsBones.Empty();
  m_CachedCameras.Empty();
  m_CachedMarkers.Empty();
  m_DataStream.Disconnect();
  return 0;
}

void FViconStreamFrameReader::Stop()
{
  m_bStopTask = true;
}

void FViconStreamFrameReader::Shutdown()
{
  if( m_pThread )
  {
    Stop();
  }
}

void FViconStreamFrameReader::Connect()
{
  if( IsConnected() )
  {
    return;
  }

  {
    if( m_pThread )
    {
      Shutdown();
      m_pThread = nullptr;
    }

    m_pThread = FRunnableThread::Create( this, TEXT( "FViconStreamFrameReader" ), 0, TPri_BelowNormal );
  }
}

FString FViconStreamFrameReader::ConstructServerAddress()
{
  FString ServerAddress;

  FString ServerName = m_ViconStreamProps.m_ServerName.ToString();
  unsigned int ServerPort = m_ViconStreamProps.m_PortNumber;

  // The server name may consist of multiple, semi-colon separated addresses.
  TArray< FString > Servers;
  if( ServerName.ParseIntoArray( Servers, TEXT( ";" ), 1 ) > 0 )
  {
    for( FString Server : Servers )
    {
      FString ServerAndPort;

      // We do not allow spaces in the input argument to the datastream connect. Remove any the
      // user may have added in their input string.
      FString TrimmedServer = Server.TrimStartAndEnd();

      // If the server address already contains a port, e.g. localhost:801, don't add the port
      if( TrimmedServer.Contains( TEXT( ":" ) ) )
      {
        ServerAndPort = TrimmedServer;
      }
      else
      {
        ServerAndPort = TrimmedServer + ":" + FString::FromInt( ServerPort );
      }

      // Build up the semi-colon separated list of servers and ports e.g
      // localhost:804;192.168.2.1:804
      if( !ServerAddress.IsEmpty() )
      {
        ServerAddress += ";";
      }

      ServerAddress += ServerAndPort;
    }
  }

  return ServerAddress;
}

void FViconStreamFrameReader::SetLightweightEnabled( bool i_bLightweight )
{
  // m_bLightweight is to work around when lightweight is set
  // before GetFrame is called. e.g. when the source setting is initialised.
  // unfortunately we can't give gui a feedback at this stage about if the
  // lightweight setting is successful, e.g. unable to tick the box if you can't
  // set it to lightweight, but you will see if its successful in the log output.
  m_bLightweight = i_bLightweight;
  m_DataStream.SetLightWeightEnabled( m_bLightweight );
}

void FViconStreamFrameReader::ShowAllVideoCamera( bool i_bShow )
{
  m_bShowAllVideoCamera = i_bShow;
}

void FViconStreamFrameReader::SetMarkerEnabled( bool i_bStreamMarker )
{
  m_DataStream.SetMarkerDataEnabled( i_bStreamMarker );
}

void FViconStreamFrameReader::SetUnlabeledMarkerEnabled( bool i_bStreamMarker )
{
  m_DataStream.SetUnlabeledMarkerDataEnabled( i_bStreamMarker );
}

void FViconStreamFrameReader::ConnectInternal()
{
  FString ServerAddress = ConstructServerAddress();
  UE_LOG( LogViconStream, Log, TEXT( "Connecting to datastream on %s" ), *ServerAddress );

  EResult ret = m_DataStream.Connect( ServerAddress, m_ViconStreamProps.m_bRetimed, m_ViconStreamProps.m_bLogOutput );

  if( ret != ESuccess )
  {
    UE_LOG( LogViconStream, Display, TEXT( "Failed to connect to datastream on %s" ), *ServerAddress );
    return;
  }

  if( m_ViconStreamProps.m_bRetimed )
  {
    m_DataStream.SetOffset( m_ViconStreamProps.m_RetimeOffset );
  }

  // Set the streaming mode appropriately
  if( m_DataStream.SetStreamMode( m_ViconStreamProps.m_bUsePrefetch ? EPullPreFetch : EPush ) != ESuccess )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Failed to set streaming mode to %s" ), ( m_ViconStreamProps.m_bUsePrefetch ? TEXT( "PreFetch" ) : TEXT( "Push" ) ) );
  }
  else
  {
    UE_LOG( LogViconStream, Display, TEXT( "Set streaming mode to %s" ), ( m_ViconStreamProps.m_bUsePrefetch ? TEXT( "PreFetch" ) : TEXT( "Push" ) ) );
  }

  // Always use scaling
  m_DataStream.SetUseScaling( m_ViconStreamProps.m_bScaled );

  // Disable Kalman for retiming for the moment
  m_DataStream.UseKalman( false );

  if( m_DataStream.GetFrame() == EResult::ESuccess )
  {
    // Set the subject filter. This must be done after connection and a frame has been received, in order
    // to determine subject IDs
    m_DataStream.SetSubjectFilter( m_ViconStreamProps.m_SubjectFilter.ToString() );
    // Called after GetFrame, to ensure that supported type information from the server has been received ( if server support lightweight)
    m_DataStream.SetLightWeightEnabled( m_bLightweight );
    if( !m_ViconStreamProps.m_SubjectFilter.IsEmpty() )
    {
      m_ViconStreamProps.m_SubjectFilter.ToString().ParseIntoArray( m_SubjectAllowed, TEXT( "," ), true );
      for( auto& Subject : m_SubjectAllowed )
      {
        Subject = Subject.TrimStartAndEnd();
      }
    }
  }
  else
  {
    UE_LOG( LogViconStream, Error, TEXT( "Failed to get frame; Subject filter could not be set" ) );
  }
}

bool FViconStreamFrameReader::IsConnected() const
{
  return m_DataStream.IsConnected();
}

//Cameras
void FViconStreamFrameReader::ClearCamerasFromLiveLink( const TSet< FString >& i_rStaleCameras )
{
  for( const auto& Camera : i_rStaleCameras )
  {
    FLiveLinkSubjectKey SubjectKey( m_SourceGuid, FName( *Camera ) );
    if( !m_bStopTask )
    {
      m_pLiveLinkClient->RemoveSubject_AnyThread( SubjectKey );
    }
    m_CachedCameras.Remove( Camera );
    UE_LOG( LogViconStream, Log, TEXT( "Removing camera %s" ), *SubjectKey.SubjectName.ToString() );
  }
}

void FViconStreamFrameReader::ClearMarkerFromLiveLink( const FLiveLinkSubjectKey& i_rLabelledMarkerKey )
{
  if( !m_bStopTask )
  {
    m_pLiveLinkClient->RemoveSubject_AnyThread( i_rLabelledMarkerKey );
  }
  m_CachedMarkers.Remove( i_rLabelledMarkerKey.SubjectName.ToString() );
  UE_LOG( LogViconStream, Log, TEXT( "Removing subject %s" ), *i_rLabelledMarkerKey.SubjectName.ToString() );
}

void FViconStreamFrameReader::HandleLabelledMarker()
{
  FString LabelledMarker( "LabelledMarker" );
  FLiveLinkSubjectKey LabelledMarkerKey( m_SourceGuid, FName( *LabelledMarker ) );

  /////////Labelled Marker Data
  {
    TArray< FVector > LabelledMarkerPoseList;
    auto Result = m_DataStream.GetLabelledMarkers( LabelledMarkerPoseList );
    unsigned int MarkerCount = LabelledMarkerPoseList.Num();
    if( 0 == MarkerCount )
    {
      ClearMarkerFromLiveLink( LabelledMarkerKey );
      return;
    }

    // Static Frame
    if( !m_CachedMarkers.Contains( LabelledMarker ) )
    {
      FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct( FLiveLinkViconMarkerStaticData::StaticStruct() );
      FLiveLinkViconMarkerStaticData& rMarkerData = *StaticDataStruct.Cast< FLiveLinkViconMarkerStaticData >();
      rMarkerData.bIsLabeled = true;
      if( m_bStopTask )
      {
        return;
      }
      m_pLiveLinkClient->PushSubjectStaticData_AnyThread( LabelledMarkerKey, ULiveLinkViconMarkerRole::StaticClass(), MoveTemp( StaticDataStruct ) );
      m_CachedMarkers.Add( LabelledMarker );
    }

    //Frame Data
    FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct( FLiveLinkViconMarkerFrameData::StaticStruct() );
    FLiveLinkViconMarkerFrameData& rMarkerFrameData = *FrameDataStruct.Cast< FLiveLinkViconMarkerFrameData >();
    rMarkerFrameData.m_MarkerData = LabelledMarkerPoseList;
    if( m_bStopTask )
    {
      return;
    }
    m_pLiveLinkClient->PushSubjectFrameData_AnyThread( LabelledMarkerKey, MoveTemp( FrameDataStruct ) );
    m_CachedMarkers[ LabelledMarker ] = MarkerCount;
  }
}

void FViconStreamFrameReader::HandleUnlabelledMarker()
{
  FString UnlabelledMarker( "UnLabelledMarker" );
  FLiveLinkSubjectKey UnlabelledMarkerKey( m_SourceGuid, FName( *UnlabelledMarker ) );

  /////////Unlabelled Marker Data
  {
    TArray< FVector > UnlabelledMarkerPoseList;
    auto Result = m_DataStream.GetUnlabelledMarkers( UnlabelledMarkerPoseList );
    unsigned int UnlabelledMarkerCount = UnlabelledMarkerPoseList.Num();
    if( 0 == UnlabelledMarkerCount )
    {
      ClearMarkerFromLiveLink( UnlabelledMarkerKey );
      return;
    }

    // Static Frame
    if( !m_CachedMarkers.Contains( UnlabelledMarker ) )
    {
      FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct( FLiveLinkViconMarkerStaticData::StaticStruct() );
      FLiveLinkViconMarkerStaticData& rMarkerData = *StaticDataStruct.Cast< FLiveLinkViconMarkerStaticData >();
      rMarkerData.bIsLabeled = false;
      if( m_bStopTask )
      {
        return;
      }
      m_pLiveLinkClient->PushSubjectStaticData_AnyThread( UnlabelledMarkerKey, ULiveLinkViconMarkerRole::StaticClass(), MoveTemp( StaticDataStruct ) );
      m_CachedMarkers.Add( UnlabelledMarker );
    }

    //Frame Data
    FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct( FLiveLinkViconMarkerFrameData::StaticStruct() );
    FLiveLinkViconMarkerFrameData& rMarkerFrameData = *FrameDataStruct.Cast< FLiveLinkViconMarkerFrameData >();
    rMarkerFrameData.m_MarkerData = UnlabelledMarkerPoseList;
    if( m_bStopTask )
    {
      return;
    }
    m_pLiveLinkClient->PushSubjectFrameData_AnyThread( UnlabelledMarkerKey, MoveTemp( FrameDataStruct ) );
    m_CachedMarkers[ UnlabelledMarker ] = UnlabelledMarkerCount;
  }
}

// todo: reference instead of the pointer
void FViconStreamFrameReader::HandleSubjectData()
{
  TArray< FString > SubjectNames;
  if( m_DataStream.GetSubjectNames( SubjectNames ) != EResult::ESuccess )
  {
    return;
  }

  // static data (skeleton)
  for( const auto& rSubject : SubjectNames )
  {
    // Bail out immediately if we're closing down
    if( m_bStopTask )
    {
      return;
    }

    if( m_SubjectAllowed.Num() != 0 && !m_SubjectAllowed.Contains( rSubject ) )
    {
      continue;
    }

    FName SubjectNameFName = FName( *rSubject );
    // If we have the subject cached, check the segments/bones hasn't changed.
    // If it changed, remove the subject.
    if( m_CachedSubjectsBones.Contains( rSubject ) )
    {
      unsigned int CachedBoneCount = m_CachedSubjectsBones[ rSubject ].Num();
      unsigned int StreamBoneCount = 0;
      if( m_DataStream.GetSegmentCountForSubject( TCHAR_TO_UTF8( *rSubject ), StreamBoneCount ) == ESuccess )
      {
        if( StreamBoneCount == CachedBoneCount )
        {
          continue;
        }
        else
        {
          UE_LOG( LogViconStream, Warning, TEXT( "Bone count changed for %s" ), *rSubject );
          if( !m_bStopTask )
          {
            m_pLiveLinkClient->RemoveSubject_AnyThread( {m_SourceGuid, SubjectNameFName} );
          }
          m_CachedSubjectsBones.Remove( rSubject );
        }
      }
    }

    // If we don't have the subject cached, we will add it below
    TArray< std::string > SubjectBones;

    bool bGotSkeleton = AddSubjectStaticDataToLiveLink( rSubject, SubjectBones );
    if( !bGotSkeleton )
    {
      UE_LOG( LogViconStream, Error, TEXT( "Failed to get Static Data for %s" ), *rSubject );
      continue;
    }
    m_CachedSubjectsBones.Add( rSubject, SubjectBones );
  }

  // frame data
  for( const auto& rSubject : SubjectNames )
  {
    if( m_SubjectAllowed.Num() != 0 && !m_SubjectAllowed.Contains( rSubject ) )
    {
      continue;
    }

    FName SubjectNameFName = FName( *rSubject );
    TArray< std::string > SubjectBones = m_CachedSubjectsBones[ rSubject ];
    FLiveLinkFrameDataStruct FrameDataStruct = ( SubjectBones.Num() == 1 ) ? FLiveLinkFrameDataStruct( FLiveLinkTransformFrameData::StaticStruct() ) : FLiveLinkFrameDataStruct( FLiveLinkAnimationFrameData::StaticStruct() );
    if( m_DataStream.GetPoseForSubject( TCHAR_TO_UTF8( *rSubject ), SubjectBones, FrameDataStruct ) )
    {
      if( !m_bStopTask )
      {
        m_pLiveLinkClient->PushSubjectFrameData_AnyThread( {m_SourceGuid, SubjectNameFName}, MoveTemp( FrameDataStruct ) );
        UE_LOG( LogViconStream, Log, TEXT( "Adding data for %s" ), *rSubject );
      }
    }
  }
}

void FViconStreamFrameReader::HandleCameraData()
{
  // get all video camera from datastream
  TSet< FString > CameraNameList;

  EResult Result;
  if( m_bShowAllVideoCamera )
  {
    Result = m_DataStream.GetVideoCameraNames( CameraNameList );
  }
  else
  {
    Result = m_DataStream.GetDynamicCameraNames( CameraNameList );
  }

  if( Result != ESuccess )
  {
    return;
  }

  TSet< FString > NotInDataStreamAnyMore = m_CachedCameras.Difference( CameraNameList );
  if( NotInDataStreamAnyMore.Num() != 0 )
  {
    ClearCamerasFromLiveLink( NotInDataStreamAnyMore );
  }

  TSet< FString > NewCameras = CameraNameList.Difference( m_CachedCameras );

  if( NewCameras.Num() != 0 )
  {
    for( const auto& rCamera : NewCameras )
    {
      FName CameraName( *rCamera );
      FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct( FLiveLinkLensStaticData::StaticStruct() );
      FLiveLinkLensStaticData& rLensData = *StaticDataStruct.Cast< FLiveLinkLensStaticData >();

      if( EError == m_DataStream.GetLensStaticData( TCHAR_TO_UTF8( *rCamera ), rLensData ) )
      {
        UE_LOG( LogViconStream, Error, TEXT( "Failed to retrieve static data for %s" ), *rCamera );
      }
      // push the data into livelink
      m_pLiveLinkClient->PushSubjectStaticData_AnyThread( {m_SourceGuid, CameraName}, ULiveLinkLensRole::StaticClass(), MoveTemp( StaticDataStruct ) );
      m_CachedCameras.Add( rCamera );
    }
  }

  // push frame data for all camera
  for( const auto& rCamera : CameraNameList )
  {
    if( m_bStopTask )
    {
      return;
    }

    FName CameraName( *rCamera );

    // camera frame data
    FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct( FLiveLinkLensFrameData::StaticStruct() );
    FLiveLinkLensFrameData& rLensData = *FrameDataStruct.Cast< FLiveLinkLensFrameData >();

    // check the result
    if( EResult::EError == m_DataStream.GetCameraTransformFrameData( TCHAR_TO_UTF8( *rCamera ), rLensData ) )
    {
      return;
    }
    if( EResult::EError == m_DataStream.GetLensFrameData( TCHAR_TO_UTF8( *rCamera ), rLensData ) )
    {
      return;
    }

    m_pLiveLinkClient->PushSubjectFrameData_AnyThread( {m_SourceGuid, CameraName}, MoveTemp( FrameDataStruct ) );
  }
}

void FViconStreamFrameReader::HandleMarkerData()
{
  HandleLabelledMarker();
  HandleUnlabelledMarker();
}

// Bind the given subject to the given skeleton and store the result.
bool FViconStreamFrameReader::AddSubjectStaticDataToLiveLink( const FString& i_rSubjectName, TArray< std::string >& o_rSubjectBones )
{
  if( m_DataStream.IsConnected() )
  {
    FName SubjectNameFName = FName( *i_rSubjectName );
    int32 NumBoneDefs = INDEX_NONE;

    unsigned int numBone = 0;
    if( m_DataStream.GetSegmentCountForSubject( TCHAR_TO_UTF8( *i_rSubjectName ), numBone ) != ESuccess )
    {
      UE_LOG( LogViconStream, Error, TEXT( "Failed to get source skeleton segment count from Vicon Stream" ) );
      return false;
    }
    // rigid body
    if( numBone == 1 )
    {
      FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct( FLiveLinkTransformStaticData::StaticStruct() );
      FLiveLinkTransformStaticData& StaticTransformData = *StaticDataStruct.Cast< FLiveLinkTransformStaticData >();
      o_rSubjectBones.Empty();

      FString Name;
      if( m_DataStream.GetSegmentNameForSubject( TCHAR_TO_UTF8( *i_rSubjectName ), 0, Name ) != ESuccess )
      {
        UE_LOG( LogViconStream, Error, TEXT( "Failed to get source skeleton segment name" ) );
        return false;
      }
      // push data
      if( !m_bStopTask )
      {
        m_pLiveLinkClient->PushSubjectStaticData_AnyThread( {m_SourceGuid, SubjectNameFName}, ULiveLinkTransformRole::StaticClass(), MoveTemp( StaticDataStruct ) );
      }

      o_rSubjectBones.Emplace( TCHAR_TO_UTF8( *Name ) );
      return true;
    }

    // subject
    FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct( FLiveLinkSkeletonStaticData::StaticStruct() );
    FLiveLinkSkeletonStaticData& StaticSkeleton = *StaticDataStruct.Cast< FLiveLinkSkeletonStaticData >();

    NumBoneDefs = static_cast< int32 >( numBone );

    StaticSkeleton.BoneNames.SetNumUninitialized( NumBoneDefs );
    StaticSkeleton.BoneParents.SetNumUninitialized( NumBoneDefs );
    o_rSubjectBones.Empty();

    // We will use a vector of strings to access the datastream, as conversion
    // to FName loses case sensitivity
    for( int32 i = 0; i < NumBoneDefs; ++i )
    {
      FString Name;

      if( m_DataStream.GetSegmentNameForSubject( TCHAR_TO_UTF8( *i_rSubjectName ), i, Name ) != ESuccess )
      {
        UE_LOG( LogViconStream, Error, TEXT( "Failed to get source skeleton segment name" ) );
        return false;
      }

      o_rSubjectBones.Emplace( TCHAR_TO_UTF8( *Name ) );
      StaticSkeleton.BoneNames[ i ] = FName( *Name );
    }

    for( int32 i = 0; i < NumBoneDefs; ++i )
    {
      FString Name;
      FString ParentName;

      if( m_DataStream.GetSegmentParentNameForSubject( TCHAR_TO_UTF8( *i_rSubjectName ), o_rSubjectBones[ i ], ParentName ) )
      {
        UE_LOG( LogViconStream, Error, TEXT( "Failed to get source skeleton segment's parent name" ) );
        return false;
      }

      int32 ParentIndex = o_rSubjectBones.Find( TCHAR_TO_UTF8( *ParentName ) );
      StaticSkeleton.BoneParents[ i ] = ParentIndex;
    }

    // push data
    if( !m_bStopTask )
    {
      m_pLiveLinkClient->PushSubjectStaticData_AnyThread( {m_SourceGuid, SubjectNameFName}, ULiveLinkAnimationRole::StaticClass(), MoveTemp( StaticDataStruct ) );
    }

    return true;
  }

  return false;
}
