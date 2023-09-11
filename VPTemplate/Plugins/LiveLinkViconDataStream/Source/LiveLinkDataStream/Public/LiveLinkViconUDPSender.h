// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "LiveLinkViconUDPSender.generated.h"

class FInternetAddr;
class FSocket;

UCLASS()
class ALiveLinkViconUDPSender : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ALiveLinkViconUDPSender();

public:
  UFUNCTION( BlueprintCallable, Category = "Vicon", meta = ( HidePin = "ToSend", ToolTip = "Send the UDP Packet to trigger Start Capture" ) )
  bool LiveLinkViconUDPStartTrigger( FString CaptureName = "TestCapture", FString CaptureNotes = "", FString CaptureDescription = "", FString DatabasePath = "C:/Users/Public/Documents/Vicon/ShogunLive1.x/Captures", FString CaptureDelayInms = "0", FString ToSend = "" );

  UFUNCTION( BlueprintCallable, Category = "Vicon", meta = ( HidePin = "ToSend", ToolTip = "Send the UDP packet to trigger Stop Capture", AdvancedDisplay = "CaptureName, CaptureNotes, CaptureDescription, DatabasePath, CaptureDelayInms" ) )
  bool LiveLinkViconUDPStopTrigger( FString CaptureName = "", FString CaptureNotes = "", FString CaptureDescription = "", FString DatabasePath = "", FString CaptureDelayInms = "0", FString ToSend = "" );

  UFUNCTION( BlueprintCallable, Category = "Vicon", meta = ( ToolTip = "Broadcast a UDP Packet with any string" ) )
  bool StartUDPSender( const FString& YourChosenSocketName,
                       FString RemoteIP = "255.255.255.255", int32 RemotePort = 30 );

  // Called whenever this actor is being removed from a level
  virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

private:
  bool SendString( FString ToSend );

  TSharedPtr< FInternetAddr > RemoteAddr;
  FSocket* SenderSocket;
};
