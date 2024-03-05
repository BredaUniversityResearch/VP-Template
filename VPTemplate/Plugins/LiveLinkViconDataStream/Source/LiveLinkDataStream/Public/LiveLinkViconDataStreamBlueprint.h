// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ILiveLinkSource.h"

#include <string>

#include "LiveLinkViconDataStreamBlueprint.generated.h"


struct FLiveLinkBasicBlueprintData;

DECLARE_LOG_CATEGORY_CLASS( LogViconDataStreamBlueprint, Display, All )

/**
 * 
 */
UCLASS()
class LIVELINKDATASTREAM_API ULiveLinkViconDataStreamBlueprint : public UBlueprintFunctionLibrary
{
public:
  static const std::string SOURCE_TYPE;

  GENERATED_BODY()

  UFUNCTION( BlueprintCallable, Category = Vicon, meta = ( DisplayName = "Create Vicon LiveLink Source", ServerName = "localhost", PortNumber = "801", SubjectFilter = "", bIsRetimed = "false", bUsePreFetch = "false", bIsScaled = "true", bLogOutput = "false", sOffset = "0.0" ) )
  static void CreateViconLiveLinkSource( FString ServerName, int32 PortNumber, FString SubjectFilter, bool bIsRetimed, bool bUsePreFetch, bool bIsScaled, bool bLogOutput, float Offset, FLiveLinkSourceHandle& SourceHandle );

  UFUNCTION( BlueprintCallable, Category = Timecode, meta = ( DisplayName = "FrameNumber To Timecode"  ) )
  static FTimecode TimecodeFromFrameNumber( const FFrameNumber& FrameNumber, const FFrameRate& FrameRate );

  /**
   * Retrieves the translation of a named marker from the provided LiveLink Basic Blueprint data.
   *
   * @param BasicData        Reference to FLiveLinkBasicBlueprintData containing marker information.
   * @param MarkerName       The name of the marker whose translation is to be retrieved.
   * @param Translation      Reference to a FVector that will store the retrieved marker translation.
   *                         If the specified marker is not found or there is an error, Translation will be (0,0,0);
   * @return                 True if the marker translation was successfully retrieved, false otherwise.
   *                         Check the result to ensure the operation was successful before using the Translation parameter.
   *                         If successful, the Translation parameter will contain the marker translation.
   */
  UFUNCTION(BlueprintPure, Category = Vicon, meta = (DisplayName = "Get Marker Translation By Name"))
  static bool GetMarkerTranslationByName(UPARAM(ref) FLiveLinkBasicBlueprintData& BasicData, FString MarkerName, FVector& Translation);

  /**
   * Retrieves marker translations from the provided LiveLink Basic Blueprint data.
   *
   * @param BasicData        Reference to FLiveLinkBasicBlueprintData containing marker information.
   * @param Translation      An array to store the retrieved marker translations.
   *                         The array will be populated with FVector values representing the translation of each marker.
   *                         In case of error, the array will be empty.
   * @return                 True if the marker translations were successfully retrieved, false otherwise.
   *                         Check the result to ensure the operation was successful before using the Translation array.
   *                         If successful, the Translation array will contain the marker translations.
   */
  UFUNCTION(BlueprintPure, Category = Vicon, meta = (DisplayName = "Get Marker Translations"))
  static bool GetMarkerTranslations(UPARAM(ref) FLiveLinkBasicBlueprintData& BasicData, TArray<FVector>& Translations);

};
