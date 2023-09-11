// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ILiveLinkSource.h"
#include "LiveLinkViconDataStreamBlueprint.generated.h"

/**
 * 
 */
UCLASS()
class LIVELINKDATASTREAM_API ULiveLinkViconDataStreamBlueprint : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

  UFUNCTION( BlueprintCallable, Category = Vicon, meta = ( DisplayName = "Create Vicon LiveLink Source", ServerName = "localhost", PortNumber = "801", SubjectFilter = "", bIsRetimed = "false", bUsePreFetch = "false", bIsScaled = "true", bLogOutput = "false", sOffset = "0.0" ) )
  static void CreateViconLiveLinkSource( FString ServerName, int32 PortNumber, FString SubjectFilter, bool bIsRetimed, bool bUsePreFetch, bool bIsScaled, bool bLogOutput, float Offset, FLiveLinkSourceHandle& SourceHandle );

  UFUNCTION( BlueprintCallable, Category = Timecode, meta = ( DisplayName = "FrameNumber To Timecode"  ) )
  static FTimecode TimecodeFromFrameNumber( const FFrameNumber& FrameNumber, const FFrameRate& FrameRate );
};
