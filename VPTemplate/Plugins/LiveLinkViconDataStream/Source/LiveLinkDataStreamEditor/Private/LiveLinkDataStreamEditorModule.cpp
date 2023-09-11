// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

#include "Interfaces/IPluginManager.h"

#include "Features/IModularFeatures.h"
#include "Misc/Paths.h"

/**
 * Implements the Messaging module.
 */

#define LOCTEXT_NAMESPACE "LiveLinkDataStreamEditorModule"

class FLiveLinkDataStreamEditorModule
: public IModuleInterface
{
public:
  // IModuleInterface interface

  virtual void StartupModule() override
  {
    // This is empty, as I don't seem to need to put anything in it, but it seems to be required to load the plugin
  }

  virtual void ShutdownModule() override
  {
  }

  virtual bool SupportsDynamicReloading() override
  {
    return false;
  }
};

IMPLEMENT_MODULE( FLiveLinkDataStreamEditorModule, LiveLinkDataStreamEditor );

#undef LOCTEXT_NAMESPACE
