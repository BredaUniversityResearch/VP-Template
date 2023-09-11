// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

// =========================================================================
// Vicon Data Stream LiveLink Source factory class.
//
// =========================================================================

#pragma once

#include "LiveLinkSourceFactory.h"
#include "ViconStreamFrameReader.h"

#include "LiveLinkViconDataStreamSourceFactory.generated.h"

class SLiveLinkViconDataStreamSourceEditor;

UCLASS()
class ULiveLinkViconDataStreamSourceFactory : public ULiveLinkSourceFactory
{
public:
  GENERATED_BODY()

  /** The name of the menu item (of any EMenuType) */
  virtual FText GetSourceDisplayName() const override;

  /** The tooltip of the menu item (of any EMenyType) */
  virtual FText GetSourceTooltip() const override;

  /**
   * How the factory should be visible in the LiveLink UI.
   * If SubPanel, BuildCreationPanel should be implemented.
   */
  virtual EMenuType GetMenuType() const { return EMenuType::SubPanel; }

  /**
   * Create a widget responsible for the creation of a Live Link source.
   * @param OnLiveLinkSourceCreated Callback to call when the source has been created by the custom UI.
   * @return The subpanel UI created by the factory.
   */
  virtual TSharedPtr< SWidget > BuildCreationPanel( FOnLiveLinkSourceCreated OnLiveLinkSourceCreated ) const override;

  /** Create a new source from a ConnectionString */
  virtual TSharedPtr< ILiveLinkSource > CreateSource( const FString& ConnectionString ) const override;

private:
  // Callback handler to take event from the editor widget and produce a source with the supplied source creation callback
  void OnPropertiesSelected( ViconStreamProperties StreamProperties, FOnLiveLinkSourceCreated OnLiveLinkSourceCreated ) const;
};
