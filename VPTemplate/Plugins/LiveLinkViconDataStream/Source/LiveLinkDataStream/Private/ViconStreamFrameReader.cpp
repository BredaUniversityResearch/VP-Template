// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.


#include "ViconStreamFrameReader.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "Roles/LiveLinkBasicRole.h"
#include "Roles/LiveLinkBasicTypes.h"
#include "ILiveLinkDataStreamModule.h"

#include "Async/Async.h"

#include "LiveLinkLensRole.h"
#include "LiveLinkLensTypes.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkCameraRole.h"
#include "Roles/LiveLinkCameraTypes.h"

const std::string FViconStreamFrameReader::UNLABELED_MARKER = "UnlabeledMarker";
const std::string FViconStreamFrameReader::LABELED_MARKER = "LabeledMarker";

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

  m_CachedSubjects.Empty();
  m_CachedCameras.Empty();
  m_CachedMarkerCounts.Empty();
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
  // Intermediate bool for same reason as m_bLightweight
  m_bMarker = i_bStreamMarker;
  m_DataStream.SetMarkerDataEnabled( m_bMarker );
}

void FViconStreamFrameReader::SetUnlabeledMarkerEnabled( bool i_bStreamMarker )
{
  // Intermediate bool for same reason as m_bLightweight
  m_bUnlabeledMarker = i_bStreamMarker;
  m_DataStream.SetUnlabeledMarkerDataEnabled( m_bUnlabeledMarker );
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
    // Called after GetFrame, to ensure that supported type information from the server has been received 
    m_DataStream.SetLightWeightEnabled( m_bLightweight );
    m_DataStream.SetMarkerDataEnabled( m_bMarker );
    m_DataStream.SetUnlabeledMarkerDataEnabled( m_bUnlabeledMarker );
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

void FViconStreamFrameReader::ClearMarkerFromLiveLink( const FLiveLinkSubjectKey& i_rMarkerKey )
{
  if( !m_bStopTask )
  {
    m_pLiveLinkClient->RemoveSubject_AnyThread( i_rMarkerKey );
  }
  m_CachedMarkerCounts.Remove( i_rMarkerKey.SubjectName.ToString() );
  UE_LOG( LogViconStream, Log, TEXT( "Removing subject %s" ), *i_rMarkerKey.SubjectName.ToString() );
}

TArray<FName> FViconStreamFrameReader::GetGenericMarkerPropertyNames(unsigned int& MarkerCount)
{
  TArray<FName> PropertyNames;
  for (unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
  {
    PropertyNames.Emplace(FString::FromInt(MarkerIndex) + "_X");
    PropertyNames.Emplace(FString::FromInt(MarkerIndex) + "_Y");
    PropertyNames.Emplace(FString::FromInt(MarkerIndex) + "_Z");
  }
  return PropertyNames;
}

void FViconStreamFrameReader::HandleMarkerData(bool bLabeled)
{
  if (m_bStopTask)
  {
    return;
  }

  FString SubjectName(bLabeled ? LABELED_MARKER.c_str() : UNLABELED_MARKER.c_str());
  FLiveLinkSubjectKey SubjectKey( m_SourceGuid, FName( *SubjectName ) );

  unsigned int MarkerCount = 0;
  const auto CountResult = bLabeled ? m_DataStream.GetLabeledMarkerCount(MarkerCount) : m_DataStream.GetUnlabeledMarkerCount(MarkerCount);
  if (CountResult == EResult::EError)
  {
    UE_LOG(LogViconStream, Warning, TEXT("Failed to get marker count for %s"), *SubjectName);
    ClearMarkerFromLiveLink(SubjectKey);
    return;
  }
  if (MarkerCount == 0)
  {
    ClearMarkerFromLiveLink(SubjectKey);
    return;
  }

  // Static data
  unsigned int* CachedMarkerCount = m_CachedMarkerCounts.Find(SubjectName);
  if (CachedMarkerCount == nullptr || *CachedMarkerCount != MarkerCount)
  {
    FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct( FLiveLinkBaseStaticData::StaticStruct() );
    FLiveLinkBaseStaticData& rMarkerStaticData = *StaticDataStruct.Cast< FLiveLinkBaseStaticData >();
    // Property names are generated from marker indices as markers are only named when associated with a Vicon subject
    rMarkerStaticData.PropertyNames = GetGenericMarkerPropertyNames(MarkerCount);
    m_pLiveLinkClient->PushSubjectStaticData_AnyThread( SubjectKey, ULiveLinkBasicRole::StaticClass(), MoveTemp( StaticDataStruct ) );
  }

  // Frame Data
  FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct( FLiveLinkBaseFrameData::StaticStruct() );
  FLiveLinkBaseFrameData& rMarkerFrameData = *FrameDataStruct.Cast< FLiveLinkBaseFrameData >();
  TArray<float>& rPropertyValues = rMarkerFrameData.PropertyValues;

  const auto TranslationResult = bLabeled ? m_DataStream.GetLabeledMarkers(rPropertyValues) : m_DataStream.GetUnlabeledMarkers(rPropertyValues);
  if (TranslationResult == EResult::EError)
  {
    UE_LOG(LogViconStream, Warning, TEXT("Failed to get markers translations for %s"), *SubjectName);
    return;
  }
  m_pLiveLinkClient->PushSubjectFrameData_AnyThread(SubjectKey, MoveTemp( FrameDataStruct ) );
  m_CachedMarkerCounts.Add(SubjectName, MarkerCount);
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
    // If we have the subject cached, check the bone count or marker count have not changed.  
    // If either did, remove the subject and re-add the static data data
    // We do this because the bone count determines whether it is a transform or an animation role
    // And the marker property names need to change when markers are enabled / disabled or the subject has been altered

    if( m_CachedSubjects.Contains( rSubject ))
    {
      const FCachedSubject& CachedSubject = m_CachedSubjects[ rSubject ];
      unsigned int StreamBoneCount = 0;
      TArray<std::string> StreamMarkerNames;
      if( m_DataStream.GetSegmentCountForSubject( TCHAR_TO_UTF8( *rSubject ), StreamBoneCount ) == ESuccess  &&
          m_DataStream.GetMarkerNamesForSubject( TCHAR_TO_UTF8( *rSubject ), StreamMarkerNames ) == ESuccess  )
      {
        // std::vector equality checks for matching lengths first so it should be efficient
        if( StreamBoneCount == CachedSubject.Bones.Num() && StreamMarkerNames == CachedSubject.Markers )
        {
          // Move on to next subject, don't need to update static data
          continue;
        }
        else
        {
          UE_LOG( LogViconStream, Warning, TEXT( "Bone count or marker names changed for %s" ), *rSubject );
          if( !m_bStopTask )
          {
            m_pLiveLinkClient->RemoveSubject_AnyThread( {m_SourceGuid, SubjectNameFName} );
          }
          m_CachedSubjects.Remove( rSubject );
        }
      }
    }

    // If we don't have the subject cached, we will add it below
    FCachedSubject CachedSubject;
    bool bGotSkeleton = AddSubjectStaticDataToLiveLink( rSubject, CachedSubject.Bones, CachedSubject.Markers);
    if ( !bGotSkeleton )
    {
      UE_LOG( LogViconStream, Error, TEXT( "Failed to get Static Data for %s" ), *rSubject );
      continue;
    }
    m_CachedSubjects.Add( rSubject, CachedSubject );
  }

  // frame data
  for ( const auto& rSubject : SubjectNames )
  {
    if ( m_SubjectAllowed.Num() != 0 && !m_SubjectAllowed.Contains(rSubject) )
    {
      continue;
    }

    FName SubjectNameFName = FName( *rSubject );
    const FCachedSubject& CachedSubject = m_CachedSubjects[ rSubject ];
    FLiveLinkFrameDataStruct FrameDataStruct = ( CachedSubject.Bones.Num() == 1 ) ?
      FLiveLinkFrameDataStruct( FLiveLinkTransformFrameData::StaticStruct() ) :
      FLiveLinkFrameDataStruct( FLiveLinkAnimationFrameData::StaticStruct() );
    if (m_DataStream.GetPoseForSubject(TCHAR_TO_UTF8(*rSubject), CachedSubject.Bones, CachedSubject.Markers, FrameDataStruct))
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
  HandleMarkerData(true);
  HandleMarkerData(false);
}

TArray<FName> FViconStreamFrameReader::MarkerPropertiesFromNames(const TArray<std::string>& i_rMarkerNames)
{
  TArray<FName> MarkerProperties;
  for (const std::string& MarkerName : i_rMarkerNames)
  {
    MarkerProperties.Emplace((MarkerName + "_X").c_str());
    MarkerProperties.Emplace((MarkerName + "_Y").c_str());
    MarkerProperties.Emplace((MarkerName + "_Z").c_str());
  }
  return MarkerProperties;
}

// Bind the given subject to the given skeleton and store the result.
bool FViconStreamFrameReader::AddSubjectStaticDataToLiveLink( const FString& i_rSubjectName, TArray< std::string >& o_rSubjectBones, TArray<std::string>& o_rMarkerNames )
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

      // markers
      if (m_DataStream.GetMarkerNamesForSubject(TCHAR_TO_UTF8(*i_rSubjectName), o_rMarkerNames) != ESuccess)
      {
        UE_LOG( LogViconStream, Error, TEXT( "Failed to get marker names for &s" ), *i_rSubjectName );
        return false;
      }
      StaticTransformData.PropertyNames = MarkerPropertiesFromNames(o_rMarkerNames);

      // skeleton segment
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
    FLiveLinkSkeletonStaticData& StaticData = *StaticDataStruct.Cast< FLiveLinkSkeletonStaticData >();

    NumBoneDefs = static_cast< int32 >( numBone );

    StaticData.BoneNames.SetNumUninitialized( NumBoneDefs );
    StaticData.BoneParents.SetNumUninitialized( NumBoneDefs );
    o_rSubjectBones.Empty();
    
    // marker names
    if (m_DataStream.GetMarkerNamesForSubject(TCHAR_TO_UTF8(*i_rSubjectName), o_rMarkerNames) != ESuccess)
    {
      UE_LOG( LogViconStream, Error, TEXT( "Failed to get marker names for &s" ), *i_rSubjectName );
      return false;
    }
    StaticData.PropertyNames = MarkerPropertiesFromNames(o_rMarkerNames);

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
      StaticData.BoneNames[ i ] = FName( *Name );
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
      StaticData.BoneParents[ i ] = ParentIndex;
    }

    // push data
    if( !m_bStopTask )
    {
      m_pLiveLinkClient->PushSubjectStaticData_AnyThread( 
          {m_SourceGuid, SubjectNameFName}, ULiveLinkAnimationRole::StaticClass(), MoveTemp( StaticDataStruct ) );
    }

    return true;
  }

  return false;
}
