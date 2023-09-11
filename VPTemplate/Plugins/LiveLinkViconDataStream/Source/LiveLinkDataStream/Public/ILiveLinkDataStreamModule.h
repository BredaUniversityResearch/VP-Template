// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

//////////////////////////////////////////////////////////////////////////
// ILiveLinkDataStreamModuleInterface

DECLARE_LOG_CATEGORY_EXTERN( LogViconLiveLink, Log, All );

class ILiveLinkDataStreamModuleInterface : public IModuleInterface
{
public:
  /**
   * Gets a reference to the data stream module instance.
   *
   * @return A reference to the data stream module.
   * @todo gmp: better implementation using dependency injection.
   */
  static ILiveLinkDataStreamModuleInterface& Get()
  {
#if PLATFORM_IOS
    static ILiveLinkDataStreamModule& LiveLinkDataStreamModule = FModuleManager::LoadModuleChecked< ILiveLinkDataStreamModule >( "LiveLinkDataStream" );
    return LiveLinkDataStreamModule;
#else
    return FModuleManager::LoadModuleChecked< ILiveLinkDataStreamModuleInterface >( "LiveLinkDataStream" );
#endif
  }

public:
  /** Virtual destructor. */
  virtual ~ILiveLinkDataStreamModuleInterface() {}
};
