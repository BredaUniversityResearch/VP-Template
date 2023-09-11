// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "LiveLinkTypes.h"
#include "Roles/LiveLinkBasicRole.h"

#include "LiveLinkViconMarkerRole.generated.h"

/**
 * Role associated for ViconMarker data.
 */
UCLASS( BlueprintType, meta = ( DisplayName = "Vicon Marker Role" ) )
class LIVELINKDATASTREAM_API ULiveLinkViconMarkerRole : public ULiveLinkBasicRole
{
  GENERATED_BODY()

public:
  //~ Begin ULiveLinkRole interface
  virtual UScriptStruct* GetStaticDataStruct() const override;
  virtual UScriptStruct* GetFrameDataStruct() const override;
  virtual UScriptStruct* GetBlueprintDataStruct() const override;

  virtual bool InitializeBlueprintData( const FLiveLinkSubjectFrameData& InSourceData, FLiveLinkBlueprintDataStruct& OutBlueprintData ) const override;

  virtual FText GetDisplayName() const override;
  //~ End ULiveLinkRole interface
};
