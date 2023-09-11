// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

// =========================================================================
// UI for LiveLink connection to a Vicon server
//
// =========================================================================

#pragma once

#include "Misc/Guid.h"
#include "ViconStreamFrameReader.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam( FOnDataStreamPropertiesSelected, ViconStreamProperties );

class SLiveLinkViconDataStreamSourceEditor : public SCompoundWidget
{
  SLATE_BEGIN_ARGS( SLiveLinkViconDataStreamSourceEditor )
  {
  }

  SLATE_EVENT( FOnDataStreamPropertiesSelected, OnPropertiesSelected )

  SLATE_END_ARGS()

  ~SLiveLinkViconDataStreamSourceEditor();

  void Construct( const FArguments& Args );

  FText GetServerName() const { return ServerName.Get()->GetText(); }
  FText GetSubjectFilter() const { return SubjectFilter.Get()->GetText(); }

  uint32 GetPortNumber() const { return PortNumber.IsSet() ? PortNumber.GetValue() : 801; }
  bool GetIsRetimed() const { return IsRetimed->IsChecked(); }
  float GetOffset() const { return Offset.IsSet() ? Offset.GetValue() : 0.0f; }
  bool GetLogOutput() const { return LogOutput->IsChecked(); }
  bool GetUsePrefetch() const { return UsePreFetch->IsChecked(); }

private:
  TSharedPtr< SEditableTextBox > ServerName;
  TSharedPtr< SEditableTextBox > SubjectFilter;
  TSharedPtr< SCheckBox > IsStreamYUp;
  TSharedPtr< SCheckBox > IsRetimed;
  TSharedPtr< SCheckBox > LogOutput;
  TSharedPtr< SCheckBox > UsePreFetch;

  TOptional< uint32 > PortNumber;
  void On_PortNumber_EntryBoxChanged( uint32 NewValue )
  {
    PortNumber = NewValue;
  }

  TOptional< uint32 > OnGet_PortNumber_EntryBoxValue() const
  {
    return PortNumber;
  }

  TOptional< float > Offset;
  void On_Offset_EntryBoxChanged( float NewValue )
  {
    Offset = NewValue;
  }

  TOptional< float > OnGet_Offset_EntryBoxValue() const
  {
    return Offset;
  }

  FReply CreateSource() const;

  FOnDataStreamPropertiesSelected OnPropertiesSelected;
};
