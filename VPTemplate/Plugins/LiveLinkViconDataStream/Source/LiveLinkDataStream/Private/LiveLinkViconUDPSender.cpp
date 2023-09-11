// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.



#include "LiveLinkViconUDPSender.h"
#include "Containers/StringConv.h"
#include "Containers/UnrealString.h"
#include "Math.h"
#include <string>

#include "Common/UdpSocketBuilder.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

// Sets default values
ALiveLinkViconUDPSender::ALiveLinkViconUDPSender()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  SenderSocket = nullptr;
}

void ALiveLinkViconUDPSender::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
  Super::EndPlay( EndPlayReason );

  if( SenderSocket )
  {
    SenderSocket->Close();
    ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->DestroySocket( SenderSocket );
  }
}

// set up the UDP socket
bool ALiveLinkViconUDPSender::StartUDPSender( const FString& YourChosenSocketName, FString RemoteIP, int32 RemotePort )
{

  //Create Remote Address.
  RemoteAddr = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();

  bool bIsValid;
  RemoteAddr->SetIp( *RemoteIP, bIsValid );
  RemoteAddr->SetPort( RemotePort );

  if( !bIsValid )
  {

    UE_LOG( LogTemp, Error, TEXT( "IP address was not valid!" ) );
    return false;
  }

  SenderSocket = FUdpSocketBuilder( *YourChosenSocketName )
                   .AsNonBlocking()
                   .AsReusable()
                   .WithBroadcast()
                   .WithSendBufferSize( 2 * 1024 * 2024 );

  //Set Send Buffer Size
  int32 SendSize = 2 * 1024 * 1024;
  SenderSocket->SetSendBufferSize( SendSize, SendSize );
  SenderSocket->SetReceiveBufferSize( SendSize, SendSize );

  UE_LOG( LogTemp, Log, TEXT( "UDP Sender Initialized Successfully using IP: %s" ), *RemoteIP );
  UE_LOG( LogTemp, Log, TEXT( "UDP Sender Initialized Successfully using Port: %d" ), RemotePort );
  return true;
}

// Send the start trigger packet
bool ALiveLinkViconUDPSender::LiveLinkViconUDPStartTrigger( FString CaptureName, FString CaptureNotes, FString CaptureDescription, FString DatabasePath, FString CaptureDelayInms, FString ToSend )
{
  if( !SenderSocket )
  {

    UE_LOG( LogTemp, Error, TEXT( "No socket" ) );
    return false;
  }

  int32 BytesSent = 0;

  //Get a random number for the packet ID to ensure 2 won't be the same
  int32 PacketID = FMath::RandRange( 0, 1000000 );
  //Convert the int into a string
  FString PacketIDStr = FString::FromInt( PacketID );

  ToSend = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><CaptureStart><Name VALUE=\"" + CaptureName + "\"/><Notes VALUE=\"" + CaptureNotes + "\"/><Description VALUE=\"" + CaptureDescription + "\"/>" + "<DatabasePath VALUE=\"" + DatabasePath + "\"/><Delay VALUE = \"" + CaptureDelayInms + "\"/><PacketID VALUE=\"" + PacketIDStr + "\"/></CaptureStart>";

  return SendString( ToSend );
}

//Send the stop trigger packet
bool ALiveLinkViconUDPSender::LiveLinkViconUDPStopTrigger( FString CaptureName, FString CaptureNotes, FString CaptureDescription, FString DatabasePath, FString CaptureDelayInms, FString ToSend )
{
  if( !SenderSocket )
  {

    UE_LOG( LogTemp, Error, TEXT( "No socket" ) );
    return false;
  }

  int32 BytesSent = 0;

  //Get a random number for the packet ID to ensure 2 won't be the same
  int32 PacketID = FMath::RandRange( 0, 1000000 );
  //Convert the int into a string
  FString PacketIDStr = FString::FromInt( PacketID );

  ToSend = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><CaptureStop><Name VALUE=\"" + CaptureName + "\"/><DatabasePath VALUE=\"" + DatabasePath + "\"/><Delay VALUE = \"" + CaptureDelayInms + "\"/><PacketID VALUE=\"" + PacketIDStr + "\"/></CaptureStop>";

  return SendString( ToSend );
}

//used for sending any string over UDP
bool ALiveLinkViconUDPSender::SendString( FString ToSend )
{
  if( !SenderSocket )
  {

    UE_LOG( LogTemp, Error, TEXT( "No Socket" ) );
    return false;
  }

  int32 BytesSent = 0;

  // Convert the FString into a TCHAR
  TCHAR* SendData = ToSend.GetCharArray().GetData();
  //find the length of the TCHAR
  int32 DataLen = FCString::Strlen( SendData );
  //send the packet using the UDP socket (also convert the TCHAR into UTF format + into an unsigned 8 bit int)
  SenderSocket->SendTo( (uint8*)TCHAR_TO_UTF8( SendData ), DataLen + 1, BytesSent, *RemoteAddr );

  if( BytesSent <= 0 )
  {
    const FString Str = "Socket is valid but 0 Bytes were sent";
    UE_LOG( LogTemp, Error, TEXT( "%s" ), *Str );
    return false;
  }

  UE_LOG( LogTemp, Log, TEXT( "UDP Send Success! Bytes Sent = %d String To Send: %s to %s " ), BytesSent, *ToSend, *RemoteAddr->ToString( true ) );

  return true;
}
