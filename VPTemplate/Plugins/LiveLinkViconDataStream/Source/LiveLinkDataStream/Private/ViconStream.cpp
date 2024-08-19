// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "ViconStream.h"

#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkTransformTypes.h"
#include "ViconLensModel.h"

#include "CommonFrameRates.h"
#include "LiveLinkLensTypes.h"
#include "Misc/Paths.h"

#include <iostream>
#include <string>


#ifdef CPP
#pragma push_macro( "CPP" )
#undef CPP
#define RESTORE_POINT_CPP
#endif

namespace
{

  static double s_RetimedFrameRate = 180.0;
  static FQuat s_YUpRotation = FQuat( FVector::XAxisVector, HALF_PI );

  void LiveLinkTimeCodeFromViconTimeCode( const ViconDataStreamSDK::CPP::Output_GetTimecode& i_rTimeCode, FQualifiedFrameTime& o_rTimeCode )
  {
    auto TimeCodeStandard = i_rTimeCode.Standard;

    switch( TimeCodeStandard )
    {
    case ViconDataStreamSDK::CPP::TimecodeStandard::PAL:
      o_rTimeCode.Rate = FCommonFrameRates::FPS_25();
      break;
    case ViconDataStreamSDK::CPP::TimecodeStandard::ATSC:
      o_rTimeCode.Rate = FCommonFrameRates::FPS_30();
      break;
    case ViconDataStreamSDK::CPP::TimecodeStandard::NTSC:
    case ViconDataStreamSDK::CPP::TimecodeStandard::NTSCDrop:
      o_rTimeCode.Rate = FCommonFrameRates::NTSC_30();
      break;
    case ViconDataStreamSDK::CPP::TimecodeStandard::Film:
      o_rTimeCode.Rate = FCommonFrameRates::FPS_24();
      break;
    case ViconDataStreamSDK::CPP::TimecodeStandard::NTSCFilm:
      o_rTimeCode.Rate = FCommonFrameRates::NTSC_24();
      break;
    default:
      break;
    }

    if( i_rTimeCode.SubFramesPerFrame > 0 )
    {
      // Set the rate to be BaseRate * SubframesPerFrames to shift subframes onto integer frames
      // i.e. 24hz with 5 subframes becomes explicitly 120hz
      o_rTimeCode.Rate.Numerator *= i_rTimeCode.SubFramesPerFrame;
    }

    FTimecode UnrealTimecode( i_rTimeCode.Hours, i_rTimeCode.Minutes, i_rTimeCode.Seconds, i_rTimeCode.Frames * i_rTimeCode.SubFramesPerFrame + i_rTimeCode.SubFrame, false );

    o_rTimeCode.Time = FFrameTime( UnrealTimecode.ToFrameNumber( o_rTimeCode.Rate ) );
  }
} // namespace

ViconStream::ViconStream()
: m_bUseScaling( true )
, m_Offset( 0.0 )
, m_bRetimed( false )
{
  m_pClient = &m_Client;

  const auto ConfigureWirelessResult = m_Client.ConfigureWireless();
  if( ConfigureWirelessResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    std::string ErrorString = ConfigureWirelessResult.Error;
    FString ErrorFString( ErrorString.c_str() );
    UE_LOG( LogViconStream, Error, TEXT( "Failed to enable wireless configuration %s" ), *ErrorFString );
  }
  else
  {
    UE_LOG( LogViconStream, Display, TEXT( "Enabled wireless configuration" ) );
  }

  m_bUseViconHMD = false;
  m_LogDebug = false;
}

ViconStream::~ViconStream()
{
  Disconnect();
}

EResult ViconStream::Connect( const FString& i_rServer, bool i_bRetimed, bool i_bLogOutput )
{
  m_ServerIP = i_rServer;
  m_bRetimed = i_bRetimed;
  m_bLogOutput = i_bLogOutput;

  FString LogDir = FPaths::ProjectLogDir();
  FString TimeDate = FDateTime::Now().ToString();

  // Timing log files
  FString TimingClientFilename = FString( "ClientTimingLog_" ).Append( TimeDate ).Append( ".csv" );
  FString TimingClientPath = FPaths::Combine( *LogDir, *TimingClientFilename );
  std::string TimingClientLog = std::string( TCHAR_TO_UTF8( *TimingClientPath ) );

  FString TimingStreamFilename = FString( "StreamTimingLog_" ).Append( TimeDate ).Append( ".csv" );
  FString TimingStreamPath = FPaths::Combine( *LogDir, *TimingStreamFilename );
  std::string TimingStreamLog = std::string( TCHAR_TO_UTF8( *TimingStreamPath ) );

  EResult ConnectionResult = EResult::EError;

  ViconDataStreamSDK::CPP::Output_Connect Result;
  if( i_bRetimed )
  {
    m_pClient = &m_RetimingClient;

    if( i_bLogOutput )
    {
      m_pClient->SetTimingLogFile( TimingClientLog, TimingStreamLog );
    }

    Result = m_RetimingClient.Connect( TCHAR_TO_UTF8( *i_rServer ), s_RetimedFrameRate );
    if( Result.Result == ViconDataStreamSDK::CPP::Result::Success || ViconDataStreamSDK::CPP::Result::ClientAlreadyConnected )
    {
      UE_LOG( LogViconStream, Display, TEXT( "Connected to retiming client %s" ), *i_rServer );
      ConnectionResult = EResult::ESuccess;
    }
  }
  else
  {
    m_pClient = &m_Client;
    Result = m_Client.Connect( TCHAR_TO_UTF8( *i_rServer ) );
    if( Result.Result == ViconDataStreamSDK::CPP::Result::Success || ViconDataStreamSDK::CPP::Result::ClientAlreadyConnected )
    {
      UE_LOG( LogViconStream, Display, TEXT( "Connected to client %s" ), *i_rServer );

      m_Client.EnableSegmentData();
      m_Client.EnableCameraCalibrationData();

      if( i_bLogOutput )
      {
        m_pClient->SetTimingLogFile( TimingClientLog, TimingStreamLog );
      }

      if( m_Client.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ServerPush ).Result == ViconDataStreamSDK::CPP::Result::Success )
      {
        ConnectionResult = EResult::ESuccess;
      }
    }
  }

  auto Output_GetVersion = m_pClient->GetVersion();
  UE_LOG( LogViconStream, Display, TEXT( "Using client version %d.%d.%d.%d" ),
          Output_GetVersion.Major, Output_GetVersion.Minor, Output_GetVersion.Point, Output_GetVersion.Revision );

  return ConnectionResult;
}

bool ViconStream::IsConnected() const
{
  const ViconDataStreamSDK::CPP::Output_IsConnected& Result = m_pClient->IsConnected();
  return Result.Connected;
}

void ViconStream::Disconnect()
{
  if( m_pClient->IsConnected().Connected )
  {
    m_pClient->Disconnect();
  }
}

unsigned int ViconStream::GetFrameNumber()
{
  if( !m_bRetimed )
  {
    return m_Client.GetFrameNumber().FrameNumber;
  }
  return 0;
}

EResult ViconStream::SetLightWeightEnabled( bool i_bEnabled )
{
  if( i_bEnabled )
  {
    auto Result = m_pClient->EnableLightweightSegmentData();
    m_Client.EnableCameraCalibrationData();

    if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
    {
      UE_LOG( LogViconStream, Display, TEXT( "Using lightweight segment data" ) );
      return EResult::ESuccess;
    }

    UE_LOG( LogViconStream, Warning, TEXT( "Unable to use lightweight segment data" ) );
    return EResult::EError;
  }

  m_pClient->DisableLightweightSegmentData();
  // DisableLightweightSegmentData will disable CameraInfo,
  // so we re-enable it by enabling camera calibration data again.
  m_Client.EnableCameraCalibrationData();
  m_Client.EnableSegmentData();
  UE_LOG( LogViconStream, Display, TEXT( "Using standard segment data" ) );
  return ESuccess;
}

void ViconStream::SetMarkerDataEnabled( bool i_bEnabled )
{
  if( i_bEnabled )
  {
    m_Client.EnableMarkerData();
  }
  else
  {
    m_Client.DisableMarkerData();
  }
}

void ViconStream::SetUnlabeledMarkerDataEnabled( bool i_bEnabled )
{
  if( i_bEnabled )
  {
    m_Client.EnableUnlabeledMarkerData();
  }
  else
  {
    m_Client.DisableUnlabeledMarkerData();
  }
}

void ViconStream::UseKalman( bool i_bEnabled )
{
  //m_RetimingClient.SetUseKalmanFilter( i_bEnabled );
}

EResult ViconStream::Reconnect()
{
  return Connect( m_ServerIP, m_bRetimed, m_bLogOutput );
}

EResult ViconStream::GetFrame()
{
  if( m_bRetimed )
  {
    // Wait for frame does not take an offset any longer
    auto Result = m_RetimingClient.WaitForFrame();
    if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
      return EResult::ESuccess;
    return EResult::EError;
  }
  else
  {
    const ViconDataStreamSDK::CPP::Output_GetFrame& Result = m_Client.GetFrame();

    if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
      return EResult::ESuccess;
    return EResult::EError;
  }
}

EResult ViconStream::SetOffset( float Offset )
{
  if( m_bRetimed )
  {
    m_Offset = Offset;

    // Running the re-timing client in push mode, we can no longer specify an offset into the future when we update frame,
    // but we can specify a negative output latency. The output mode applies this latency to the timestamps that it calculates,
    // so this has the effect of pushing the requested time in to the future.
    m_RetimingClient.SetOutputLatency( -Offset );

    return ESuccess;
  }
  else
  {
    return EError;
  }
}

void ViconStream::SetUseScaling( bool i_bUseScaling )
{
  m_bUseScaling = i_bUseScaling;
}

EResult ViconStream::SetSubjectFilter( const FString& i_rSubjectFilter )
{
  m_pClient->ClearSubjectFilter();
  TArray< FString > Subjects;
  i_rSubjectFilter.ParseIntoArray( Subjects, TEXT( "," ), true );
  for( const FString& Subject : Subjects )
  {
    FString TrimmedSubject = Subject.TrimStartAndEnd();
    ViconDataStreamSDK::CPP::Output_AddToSubjectFilter Output = m_pClient->AddToSubjectFilter( TCHAR_TO_UTF8( *TrimmedSubject ) );
    if( Output.Result != ViconDataStreamSDK::CPP::Result::Success )
    {
      UE_LOG( LogViconStream, Error, TEXT( "Failed to set subject filter for %s" ), *TrimmedSubject );
      return EError;
    }
  }
  return ESuccess;
}

EResult ViconStream::SetStreamMode( const EStreamMode& i_rMode )
{
  if( IsRetimed() )
  {
    // Retiming client currently only supports Push
    if( i_rMode == EPush )
    {
      return ESuccess;
    }
    else
    {
      return EError;
    }
  }
  else
  {
    ViconDataStreamSDK::CPP::StreamMode::Enum Mode;
    switch( i_rMode )
    {
    case EPush:
      Mode = ViconDataStreamSDK::CPP::StreamMode::ServerPush;
      break;
    case EPullPreFetch:
      Mode = ViconDataStreamSDK::CPP::StreamMode::ClientPullPreFetch;
      break;
    case EPull:
      Mode = ViconDataStreamSDK::CPP::StreamMode::ClientPull;
      break;
    default:
      return EError;
    }

    return ( m_Client.SetStreamMode( Mode ).Result == ViconDataStreamSDK::CPP::Result::Success ? ESuccess : EError );
  }
}

void ViconStream::SetAxisMappingOculus()
{
  m_pClient->SetAxisMapping( ViconDataStreamSDK::CPP::Direction::Right, ViconDataStreamSDK::CPP::Direction::Up, ViconDataStreamSDK::CPP::Direction::Backward );
}

void ViconStream::SetAxisMappingDefault()
{
  m_pClient->SetAxisMapping( ViconDataStreamSDK::CPP::Direction::Forward, ViconDataStreamSDK::CPP::Direction::Left, ViconDataStreamSDK::CPP::Direction::Up );
}

EResult ViconStream::GetSegmentCountForSubject( const std::string& i_rSubjectNme, unsigned int& o_rCount ) const
{
  o_rCount = 0;
  const ViconDataStreamSDK::CPP::Output_GetSegmentCount& Result = m_pClient->GetSegmentCount( i_rSubjectNme );
  if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
  {
    o_rCount = Result.SegmentCount;
    return EResult::ESuccess;
  }

  if( Result.Result == ViconDataStreamSDK::CPP::Result::NoFrame )
  {
    UE_LOG( LogViconStream, Error, TEXT( "NoFrame" ) );
  }
  else if( Result.Result == ViconDataStreamSDK::CPP::Result::InvalidSubjectName )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Invalid Subject Name" ) );
  }
  return EResult::EError;
}

EResult ViconStream::GetSegmentNameForSubject( const std::string& i_rSubjectNme, int Index, FString& o_rSegName ) const
{
  const ViconDataStreamSDK::CPP::Output_GetSegmentName& Result = m_pClient->GetSegmentName( i_rSubjectNme, Index );

  if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
  {
    o_rSegName = std::string( Result.SegmentName ).c_str();
    return EResult::ESuccess;
  }

  if( Result.Result == ViconDataStreamSDK::CPP::Result::InvalidSubjectName )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Invalid Subject name" ) );
  }
  return EResult::EError;
}

EResult ViconStream::GetSegmentParentNameForSubject( const std::string& i_rSubjectName, const std::string& i_rSegName, FString& o_rSegName ) const
{
  // We've removed use of local string copies here as it seems to cause corruption when run as a packaged build. There may be a bug in the Datastream String class that
  // will take further investigation.
  const ViconDataStreamSDK::CPP::Output_GetSegmentParentName& Result = m_pClient->GetSegmentParentName( i_rSubjectName, i_rSegName );

  if( Result.Result == ViconDataStreamSDK::CPP::Result::InvalidSubjectName )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Invalid Subject name" ) );
  }

  if( Result.Result == ViconDataStreamSDK::CPP::Result::Success || Result.Result == ViconDataStreamSDK::CPP::Result::Unknown )
  {
    o_rSegName = std::string( Result.SegmentName ).c_str();
    return EResult::ESuccess;
  }

  return EResult::EError;
}

EResult ViconStream::GetSegmentLocalPose( const std::string& i_rSubjectName, const std::string& i_rSegmentName, FTransform& o_rPose )
{
  // Scale

  const ViconDataStreamSDK::CPP::Output_GetSegmentStaticScale& SegScale = m_pClient->GetSegmentStaticScale( i_rSubjectName, i_rSegmentName );
  if( m_bUseScaling && SegScale.Result == ViconDataStreamSDK::CPP::Result::Success )
  {
    UE_LOG( LogViconStream, Log, TEXT( "Using subject scale of %f %f %f" ), SegScale.Scale[ 0 ], SegScale.Scale[ 1 ], SegScale.Scale[ 2 ] );
    o_rPose.SetScale3D( FVector( SegScale.Scale[ 0 ], SegScale.Scale[ 1 ], SegScale.Scale[ 2 ] ) );
  }
  else
  {
    // TODO: If we want to scale per bone correctly, we will have to apply the inverse of the parents transformation first.
    // We will probably do this at the server side when we set the transformations; i.e. scale is relative to its parent scale.
    o_rPose.SetScale3D( FVector( 1.0, 1.0, 1.0 ) );
  }

  //Translation
  const ViconDataStreamSDK::CPP::Output_GetSegmentLocalTranslation& SegLocalTranslation = m_pClient->GetSegmentLocalTranslation( i_rSubjectName, i_rSegmentName );
  std::pair< std::string, std::string > CachedSegment( i_rSubjectName, i_rSegmentName );

  if( SegLocalTranslation.Result != ViconDataStreamSDK::CPP::Result::Success || SegLocalTranslation.Occluded )
  {
    if( m_CachedSubject.count( CachedSegment ) )
    {
      o_rPose = m_CachedSubject[ CachedSegment ];
      UE_LOG( LogViconStream, Log, TEXT( "Segment is occluded, using cached data" ) );
      return ESuccess;
    }
    else
    {
      UE_LOG( LogViconStream, Log, TEXT( "Segment is occluded and there's no cached data either" ) );
      o_rPose.SetTranslation( FVector( 0, 0, 0 ) );
      return EError;
    }
  }
  else
  {
    FVector Translation = FVector( SegLocalTranslation.Translation[ 0 ], -SegLocalTranslation.Translation[ 1 ], SegLocalTranslation.Translation[ 2 ] ) * 0.1;
    FVector SegmentScale;
    FVector ScaledTranslation;
    if( m_bUseScaling && GetSegmentScale( i_rSubjectName, i_rSegmentName, SegmentScale ) == EResult::ESuccess )
    {
      for( unsigned int i = 0; i < 3; ++i )
      {
        if( SegmentScale[ i ] != 0 )
        {
          ScaledTranslation[ i ] = Translation[ i ] / SegmentScale[ i ];
        }
        else
        {
          ScaledTranslation[ i ] = Translation[ i ];
        }
      }
    }
    else
    {
      ScaledTranslation = Translation;
    }

    o_rPose.SetTranslation( ScaledTranslation );
  }

  //Rotation
  const ViconDataStreamSDK::CPP::Output_GetSegmentLocalRotationQuaternion& SegLocalRotation = m_pClient->GetSegmentLocalRotationQuaternion( i_rSubjectName, i_rSegmentName );
  if( SegLocalRotation.Rotation[ 3 ] == 0 )
  {
    // todo: work out where the (0,0,0,0) is from.
    return EError;
  }
  if( SegLocalRotation.Result != ViconDataStreamSDK::CPP::Result::Success || SegLocalRotation.Occluded )
  {
    // shouldn't hit here
    return EError;
  }
  else
  {
    o_rPose.SetRotation( FQuat( -SegLocalRotation.Rotation[ 0 ], SegLocalRotation.Rotation[ 1 ], -SegLocalRotation.Rotation[ 2 ], SegLocalRotation.Rotation[ 3 ] ) );
  }

  m_CachedSubject[ CachedSegment ] = o_rPose;

  return ESuccess;
}

EResult ViconStream::GetSubjectCount( int& o_rCount ) const
{
  const ViconDataStreamSDK::CPP::Output_GetSubjectCount& Result = m_pClient->GetSubjectCount();
  if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
  {
    o_rCount = Result.SubjectCount;
    return EResult::ESuccess;
  }
  return EResult::EError;
}

EResult ViconStream::GetSubjectName( int Index, FString& o_rName ) const
{
  const ViconDataStreamSDK::CPP::Output_GetSubjectName& Result = m_pClient->GetSubjectName( Index );
  if( Result.Result == ViconDataStreamSDK::CPP::Result::Success )
  {
    o_rName = std::string( Result.SubjectName ).c_str();
    return EResult::ESuccess;
  }
  return EResult::EError;
}

EResult ViconStream::GetSubjectNames( TArray< FString >& SubjectNames )
{
  int SubjectCount;
  EResult CountResult = GetSubjectCount( SubjectCount );
  if( CountResult != EResult::ESuccess )
  {
    UE_LOG( LogViconStream, Log, TEXT( "Failed to get Subject Names in stream" ) );
    return EResult::EError;
  }

  for( int SubjectIndex = 0; SubjectIndex < SubjectCount; ++SubjectIndex )
  {
    FString SubjectName;
    EResult SubjectNameResult = GetSubjectName( SubjectIndex, SubjectName );
    if( SubjectNameResult != EResult::ESuccess )
    {
      return EResult::EError;
    }
    SubjectNames.Emplace( SubjectName );
  }
  return EResult::ESuccess;
}

EResult ViconStream::GetRootPose( const std::string& i_rSubjectName, FVector& o_rPosition, FQuat& o_rOrientation )
{
  auto NameResult = m_pClient->GetSubjectRootSegmentName( i_rSubjectName );
  if( NameResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Can't get root segment name" ) );
    return EResult::EError;
  }

  ViconDataStreamSDK::CPP::String RootName = NameResult.SegmentName;
  FTransform Pose;

  EResult Result = GetSegmentLocalPose( i_rSubjectName, std::string( RootName ).c_str(), Pose );

  o_rPosition = Pose.GetTranslation();
  o_rOrientation = Pose.GetRotation();
  return Result;
}

EResult ViconStream::GetDynamicCameraCount( int& o_rCount ) const
{
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Camera data doesn't support retime mode" ) );
    return EResult::EError;
  }

  auto CameraCountResult = m_Client.GetDynamicCameraCount();
  if( CameraCountResult.Result == ViconDataStreamSDK::CPP::Result::Success )
  {
    o_rCount = CameraCountResult.CameraCount;
    return EResult::ESuccess;
  }
  return EResult::EError;
}

EResult ViconStream::GetDynamicCameraNames( TSet< FString >& o_rNameList ) const
{
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Camera data doesn't support retime mode" ) );
    return EResult::EError;
  }
  o_rNameList.Empty();

  int CameraCount = 0;
  auto Result = GetDynamicCameraCount( CameraCount );
  for( int CameraIndex = 0; CameraIndex < CameraCount; ++CameraIndex )
  {
    auto CameraNameResult = m_Client.GetDynamicCameraName( CameraIndex );
    if( CameraNameResult.Result != ViconDataStreamSDK::CPP::Result::Success )
    {
      //UE_LOG( LogViconStream, Error, TEXT( "Failed to retrieve camera name" ) );
      return EResult::EError;
    }


    o_rNameList.Add( std::string( CameraNameResult.CameraName ).c_str() );
  }
  return EResult::ESuccess;
}

EResult ViconStream::GetVideoCameraNames( TSet< FString >& o_rNameList ) const
{
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Camera data doesn't support retime mode" ) );
    return EResult::EError;
  }
  o_rNameList.Empty();
  auto CameraCountResult = m_Client.GetCameraCount();
  for( unsigned int CameraIndex = 0; CameraIndex < CameraCountResult.CameraCount; ++CameraIndex )
  {
    auto CameraNameResult = m_Client.GetCameraName( CameraIndex );

    if( m_Client.GetIsVideoCamera( CameraNameResult.CameraName ).IsVideoCamera )
    {
      o_rNameList.Add( std::string( CameraNameResult.CameraName ).c_str() );
    }
  }
  return EResult::ESuccess;
}

EResult ViconStream::GetCameraTransformFrameData( const std::string& i_rCameraName, FLiveLinkTransformFrameData& OutSubject )
{
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Camera data doesn't support retime mode" ) );
    return EResult::EError;
  }

  auto TranslationResult = m_Client.GetCameraGlobalTranslation( i_rCameraName );
  if( TranslationResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    return EResult::EError;
  }
  // mapping to unreal by mirroring xz plane
  FVector Translation = FVector( TranslationResult.Translation[ 0 ], -TranslationResult.Translation[ 1 ], TranslationResult.Translation[ 2 ] ) * 0.1;
  OutSubject.Transform.SetTranslation( Translation );

  auto RotationResult = m_Client.GetCameraGlobalRotationQuaternion( i_rCameraName );
  if( RotationResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    return EResult::EError;
  }
  FQuat Rotation = FQuat( -RotationResult.Rotation[ 0 ], RotationResult.Rotation[ 1 ], -RotationResult.Rotation[ 2 ], RotationResult.Rotation[ 3 ] );
  OutSubject.Transform.SetRotation( Rotation );

  // server axis mapping
  if( IsViconServerYup() )
  {
    OutSubject.Transform = OutSubject.Transform * s_YUpRotation;
  }

  // Extra camera rotation as it's ( right, down, forward )in data stream
  FQuat CameraRotation = OutSubject.Transform.GetRotation();
  FMatrix RotMatrix( FVector::ZAxisVector, FVector::XAxisVector, FVector::YAxisVector, FVector::ZeroVector );
  OutSubject.Transform.SetRotation( FQuat( CameraRotation * RotMatrix ) );

  return EResult::ESuccess;
}

EResult ViconStream::GetLensStaticData( const std::string& i_rCameraName, FLiveLinkLensStaticData& LensStaticData )
{
  LensStaticData.LensModel = UViconLensModel::LensModelName;
  return ESuccess;
}

EResult ViconStream::GetLensFrameData( const std::string& i_rCameraName, FLiveLinkLensFrameData& LensFrameData )
{
  // not available in retimed data
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Camera data doesn't support retime mode." ) );
    return EResult::EError;
  }
  //
  auto ResolutionResult = m_Client.GetCameraResolution( i_rCameraName );
  if( ResolutionResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Couldn't get camera resolution." ) );
    return EResult::EError;
  }
  FVector2D Resolution = FVector2D( ResolutionResult.ResolutionX, ResolutionResult.ResolutionY );

  //
  auto FocalLengthResult = m_Client.GetCameraFocalLength( i_rCameraName );
  if( FocalLengthResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Couldn't get camera focal length." ) );
    return EResult::EError;
  }
  auto FocalLength = FocalLengthResult.FocalLength;

  // param
  auto ParamResult = m_Client.GetCameraLensParameters( i_rCameraName );
  if( ParamResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Couldn't get camera lens parameters." ) );
    return EResult::EError;
  }

  // principal point
  auto PrinciplePointResult = m_Client.GetCameraPrincipalPoint( i_rCameraName );
  if( PrinciplePointResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    UE_LOG( LogViconStream, Error, TEXT( "Couldn't get camera principal point." ) );
    return EResult::EError;
  }

  // adjust it to the unreal spherical modal
  float ViconFocalP2 = FMath::Pow( FocalLength, 2 );
  float ViconFocalP4 = FMath::Pow( FocalLength, 4 );
  float ViconFocalP6 = FMath::Pow( FocalLength, 6 );

  LensFrameData.DistortionParameters = {(float)ParamResult.LensParameters[ 0 ] * ViconFocalP2,
                                        (float)ParamResult.LensParameters[ 1 ] * ViconFocalP4,
                                        (float)ParamResult.LensParameters[ 2 ] * ViconFocalP6};

  LensFrameData.PrincipalPoint = FVector2D( PrinciplePointResult.PrincipalPointX, PrinciplePointResult.PrincipalPointY ) / Resolution;
  LensFrameData.FxFy = FVector2D( FocalLength / Resolution.X, FocalLength / Resolution.Y );

  // Add timecode to metadata
  ViconDataStreamSDK::CPP::Output_GetTimecode GetTimeCodeResult = m_Client.GetTimecode();
  if( GetTimeCodeResult.Result == ViconDataStreamSDK::CPP::Result::Success && GetTimeCodeResult.SubFramesPerFrame > 0 )
  {
    LiveLinkTimeCodeFromViconTimeCode( GetTimeCodeResult, LensFrameData.MetaData.SceneTime );
  }

  return EResult::ESuccess;
}

FVector ViconStream::HandleMarker(const double i_Translation[3])
{
  FTransform MarkerTransformation;
  MarkerTransformation.SetRotation(FQuat::Identity);
  // 0.1 for mm->cm conversion
  MarkerTransformation.SetTranslation(FVector(i_Translation[0], -i_Translation[1], i_Translation[2]) * 0.1);
  if (IsViconServerYup())
  {
    MarkerTransformation = MarkerTransformation * s_YUpRotation;
  }
  auto Marker = MarkerTransformation.GetTranslation();
  return FVector { Marker.X, Marker.Y, Marker.Z };
}

EResult ViconStream::GetUnlabeledMarkerCount(unsigned int& o_rCount)
{
  o_rCount = 0;
  if (!m_Client.IsUnlabeledMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }
  const auto Result = m_Client.GetUnlabeledMarkerCount();
  o_rCount = Result.MarkerCount;
  return Result.Result ? EResult::ESuccess : EResult::EError;
}

EResult ViconStream::GetLabeledMarkerCount(unsigned int& o_rCount)
{
  o_rCount = 0;
  if (!m_Client.IsMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }
  const auto Result = m_Client.GetLabeledMarkerCount();
  o_rCount = Result.MarkerCount;
  return Result.Result ? EResult::ESuccess : EResult::EError;
}

EResult ViconStream::GetLabeledMarkers( TArrayView< float >& o_rMarkerList )
{
  // not available in retimed data
  if (!m_Client.IsMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Marker data doesn't support retime mode" ) );
    return EResult::EError;
  }

  unsigned int MarkerCount = m_Client.GetLabeledMarkerCount().MarkerCount;
  for( unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex )
  {
    const auto Result = m_Client.GetLabeledMarkerGlobalTranslation( MarkerIndex );
    const auto MarkerPose = HandleMarker(Result.Translation);
    o_rMarkerList[MarkerIndex*3] = MarkerPose[0];
    o_rMarkerList[MarkerIndex*3+1]= MarkerPose[1];
    o_rMarkerList[MarkerIndex*3+2] = MarkerPose[2];
  }
  return EResult::ESuccess;
}

EResult ViconStream::GetUnlabeledMarkers(TArrayView < float > & o_rMarkerList)
{
  // not available in retimed data
  if (!m_Client.IsUnlabeledMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Marker data doesn't support retime mode" ) );
    return EResult::EError;
  }

  unsigned int MarkerCount = m_Client.GetUnlabeledMarkerCount().MarkerCount;
  for( unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex )
  {
    const auto Result = m_Client.GetUnlabeledMarkerGlobalTranslation(MarkerIndex);
    const auto MarkerPose = HandleMarker(Result.Translation);
    o_rMarkerList[MarkerIndex*3] = MarkerPose[0];
    o_rMarkerList[MarkerIndex*3+1]= MarkerPose[1];
    o_rMarkerList[MarkerIndex*3+2] = MarkerPose[2];
  }
  return EResult::ESuccess;
}

EResult ViconStream::GetMarkerCountForSubject(const std::string& i_rSubjectName, unsigned int& o_rCount)
{
  o_rCount = 0;
  if (!m_Client.IsMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }
  const auto Result = m_Client.GetMarkerCount(i_rSubjectName);
  o_rCount = Result.MarkerCount;
  return Result.Result ? EResult::ESuccess : EResult::EError;
}

EResult ViconStream::GetMarkerNamesForSubject(const std::string& i_rSubjectName, TArray<std::string>& o_rNames)
{
  o_rNames.Empty();
  if (!m_Client.IsMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }

  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Marker data doesn't support retime mode" ) );
    return EResult::EError;
  }
  
  const auto CountResult = m_Client.GetMarkerCount(i_rSubjectName);
  if (!CountResult.Result)
  {
    return EResult::EError;
  }
  for (unsigned int MarkerIndex = 0; MarkerIndex < CountResult.MarkerCount; MarkerIndex++)
  {
    const auto Result = m_Client.GetMarkerName(i_rSubjectName, MarkerIndex);
    if (!Result.Result)
    {
      return EResult::EError;
    }
    o_rNames.Emplace(Result.MarkerName);
  }
  return EResult::ESuccess;
}

EResult ViconStream::GetMarkersForSubject(const std::string& i_rSubjectName, const TArray<std::string>& i_rMarkerNames, TArray<float>& o_rMarkerValues) 
{
  o_rMarkerValues.Empty();
  o_rMarkerValues.Emplace(static_cast<float>(i_rMarkerNames.Num()));
  if (!m_Client.IsMarkerDataEnabled().Enabled)
  {
    return EResult::ESuccess;
  }
  if( m_bRetimed )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Marker data doesn't support retime mode" ) );
    return EResult::EError;
  }

  for (const std::string& rMarkerName: i_rMarkerNames)
  {
    const auto TransformResult = m_Client.GetMarkerGlobalTranslation(i_rSubjectName, rMarkerName);
    if (!TransformResult.Result)
    {
      return EResult::EError;
    }
    const FVector MarkerTranslation = HandleMarker(TransformResult.Translation);
    o_rMarkerValues.Emplace(MarkerTranslation.X);
    o_rMarkerValues.Emplace(MarkerTranslation.Y);
    o_rMarkerValues.Emplace(MarkerTranslation.Z);
  }
  return EResult::ESuccess;
}

bool ViconStream::GetPoseForSubject( 
  const std::string& InName, const TArray< std::string >& BoneNames, const TArray<std::string>& MarkerNames, FLiveLinkFrameDataStruct& OutSubject )
{
  ViconDataStreamSDK::CPP::Output_GetSegmentCount SegmentCount = m_pClient->GetSegmentCount( InName );
  if( SegmentCount.Result != ViconDataStreamSDK::CPP::Result::Success )
    return false;

  // rigid body
  if( SegmentCount.SegmentCount == 1 )
  {
    FLiveLinkTransformFrameData& FrameData = *OutSubject.Cast< FLiveLinkTransformFrameData >();
    FTransform& Pose = FrameData.Transform;
    if( GetSegmentLocalPose( InName, BoneNames[ 0 ], Pose ) != EResult::ESuccess )
    {
      UE_LOG( LogViconStream, Log, TEXT( "Failed to get Segment for %hs:%hs" ),
              InName.c_str(), BoneNames[ 0 ].c_str() );
      return false;
    }
    if( IsViconServerYup() )
    {
      Pose = Pose * s_YUpRotation;
    }

    ViconDataStreamSDK::CPP::Output_GetTimecode GetTimeCodeResult = m_Client.GetTimecode();
    if( GetTimeCodeResult.Result == ViconDataStreamSDK::CPP::Result::Success && GetTimeCodeResult.SubFramesPerFrame > 0 )
    {
      LiveLinkTimeCodeFromViconTimeCode( GetTimeCodeResult, FrameData.MetaData.SceneTime );
    }
    GetMarkersForSubject(InName, MarkerNames, FrameData.PropertyValues);
    return true;
  }

  //subject
  FLiveLinkAnimationFrameData& FrameData = *OutSubject.Cast< FLiveLinkAnimationFrameData >();
  TArray< FTransform >& OutPose = FrameData.Transforms;

  OutPose.SetNumZeroed( SegmentCount.SegmentCount );

  unsigned int BoneCount = (unsigned int)BoneNames.Num();
  if( SegmentCount.SegmentCount != BoneCount )
  {
    UE_LOG( LogViconStream, Warning, TEXT( "Vicon segments has %d segments while Livelink skeleton has %d bones" ),
            SegmentCount.SegmentCount, BoneCount );
  }
  unsigned int Available = SegmentCount.SegmentCount < BoneCount ? SegmentCount.SegmentCount : BoneCount;
  for( unsigned int j = 0; j < Available; ++j )
  {
    FTransform Trans = OutPose[ j ];
    if( GetSegmentLocalPose( InName, BoneNames[ j ], Trans ) != EResult::ESuccess )
    {
      UE_LOG( LogViconStream, Log, TEXT( "Failed to get Segment for %hs:%hs" ),
              InName.c_str(), BoneNames[ j ].c_str() );
      return false;
    }

    OutPose[ j ] = Trans;
  }

  if( IsViconServerYup() )
  {
    OutPose[ 0 ] = OutPose[ 0 ] * s_YUpRotation;
  }

  ViconDataStreamSDK::CPP::Output_GetTimecode GetTimeCodeResult = m_Client.GetTimecode();
  if( GetTimeCodeResult.Result == ViconDataStreamSDK::CPP::Result::Success && GetTimeCodeResult.SubFramesPerFrame > 0 )
  {
    LiveLinkTimeCodeFromViconTimeCode( GetTimeCodeResult, FrameData.MetaData.SceneTime );
  }
  GetMarkersForSubject(InName, MarkerNames, FrameData.PropertyValues);

  return true;
}

EResult ViconStream::GetSegmentScale( const std::string& i_rSubjectName, const std::string& i_rSegmentName, FVector& o_rScale )
{
  o_rScale = FVector( 1.0, 1.0, 1.0 );

  ViconDataStreamSDK::CPP::Output_GetSegmentParentName ParentNameResult;

  // First check if the input segment has a parent. If it doesn't, we don't want to report a scale, as
  // we don't want to scale the root node translation, so just return immediately.
  ParentNameResult = m_pClient->GetSegmentParentName( i_rSubjectName, i_rSegmentName );
  if( ParentNameResult.Result != ViconDataStreamSDK::CPP::Result::Success )
  {
    return EResult::ESuccess;
  }

  // Otherwise, iterate up the hierarchy, multiplying together the scales for all of the bones including the root.
  std::string SegmentName( i_rSegmentName );
  bool bHasParent = false;
  do
  {
    // Get the scale for this segment. We only want to apply scale to segments with a parent; we do not want to scale the root segment position
    const ViconDataStreamSDK::CPP::Output_GetSegmentStaticScale& SegScale = m_pClient->GetSegmentStaticScale( i_rSubjectName, SegmentName );
    if( SegScale.Result == ViconDataStreamSDK::CPP::Result::Success )
    {
      o_rScale = o_rScale * FVector( SegScale.Scale[ 0 ], SegScale.Scale[ 1 ], SegScale.Scale[ 2 ] );
    }

    // Get the segments parent
    auto ParentResult = m_pClient->GetSegmentParentName( i_rSubjectName, SegmentName );
    bHasParent = ( ParentResult.Result == ViconDataStreamSDK::CPP::Result::Success );
    if( bHasParent )
    {

      SegmentName = ParentResult.SegmentName;
    }
  } while( bHasParent );

  return EResult::ESuccess;
}

bool ViconStream::IsViconServerYup()
{
  // todo: Retiming client doesn't have server orientation
  auto ServerOrientation = m_Client.GetServerOrientation();
  bool bYUp = ( ServerOrientation.Result == ViconDataStreamSDK::CPP::Result::Success && ServerOrientation.Orientation == ViconDataStreamSDK::CPP::ServerOrientation::YUp );

  return bYUp;
}

#ifdef RESTORE_POINT_CPP
#pragma pop_macro( "CPP" )
#undef RESTORE_POINT_CPP
#endif
