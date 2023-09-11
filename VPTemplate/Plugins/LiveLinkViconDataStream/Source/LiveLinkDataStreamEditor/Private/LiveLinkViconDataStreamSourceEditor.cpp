// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.

#include "LiveLinkViconDataStreamSourceEditor.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "LiveLinkViconDataStreamSourceEditor"

SLiveLinkViconDataStreamSourceEditor::~SLiveLinkViconDataStreamSourceEditor()
{
}

void SLiveLinkViconDataStreamSourceEditor::Construct( const FArguments& Args )
{
  //  CurrentPollRequest = FGuid::NewGuid();
  OnPropertiesSelected = Args._OnPropertiesSelected;

  PortNumber = 801;
  Offset = 0.0f;

  ChildSlot
    [ SNew( SBox )
    .HeightOverride(215)
        .WidthOverride( 250 )
          [ SNew( SVerticalBox ) + SVerticalBox::Slot().AutoHeight().Padding( 2.0f )[ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SNew( STextBlock ).Text( LOCTEXT( "ViconServerName", "Vicon Server Name" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Fill ).FillWidth( 0.5f )[ SAssignNew( ServerName, SEditableTextBox ).Text( LOCTEXT( "UndeterminedViconServerName", "localhost" ) ) ] ] + SVerticalBox::Slot().AutoHeight().Padding( 2.0f )[ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SNew( STextBlock ).Text( LOCTEXT( "ViconPortNumber", "Port Number" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Fill ).FillWidth( 0.5f )[ SNew( SNumericEntryBox< uint32 > ).Value( this, &SLiveLinkViconDataStreamSourceEditor::OnGet_PortNumber_EntryBoxValue ).OnValueChanged( this, &SLiveLinkViconDataStreamSourceEditor::On_PortNumber_EntryBoxChanged ) ] ] + SVerticalBox::Slot().AutoHeight().Padding( 8.0f, 4.0f, 8.0f, 4.0f )[ SNew( SSeparator ) ] + SVerticalBox::Slot().AutoHeight().Padding( 2.0f )[ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SNew( STextBlock ).Text( LOCTEXT( "UsePreFetch", "Use PreFetch" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SAssignNew( UsePreFetch, SCheckBox ).IsChecked( ECheckBoxState::Unchecked ) ] ] + SVerticalBox::Slot().AutoHeight().Padding( 2.0f )[ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SNew( STextBlock ).Text( LOCTEXT( "IsRetimed", "Is Retimed" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SAssignNew( IsRetimed, SCheckBox ).IsChecked( ECheckBoxState::Unchecked ) ] ] + SVerticalBox::Slot().AutoHeight().Padding( 2.0f )[ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.1f )[ SNew( SBox ) ] + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.4f )[ SNew( STextBlock ).Text( LOCTEXT( "Offset", "Offset" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Fill ).FillWidth( 0.5f )[ SNew( SNumericEntryBox< float > ).Value( this, &SLiveLinkViconDataStreamSourceEditor::OnGet_Offset_EntryBoxValue ).OnValueChanged( this, &SLiveLinkViconDataStreamSourceEditor::On_Offset_EntryBoxChanged ) ] ] + SVerticalBox::Slot().AutoHeight().Padding( 2.0f )

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 [ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SNew( STextBlock ).Text( LOCTEXT( "LogOutput", "Log Output" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SAssignNew( LogOutput, SCheckBox ).IsChecked( ECheckBoxState::Unchecked ) ] ] +
            SVerticalBox::Slot()
              .AutoHeight()
              .Padding( 2.0f )
                [ SNew( SHorizontalBox ) + SHorizontalBox::Slot().HAlign( HAlign_Left ).FillWidth( 0.5f )[ SNew( STextBlock ).Text( LOCTEXT( "Subject Filter", "Subject Filter" ) ) ] + SHorizontalBox::Slot().HAlign( HAlign_Fill ).FillWidth( 0.5f )[ SAssignNew( SubjectFilter, SEditableTextBox ).Text( LOCTEXT( "EmptyFilter", "" ) ) ] ] +
            SVerticalBox::Slot()
              .AutoHeight()
              .Padding( 2.0f )
                [ SNew( SButton )
                    .Text( LOCTEXT( "Create", "Create" ) )
                    .OnClicked( this, &SLiveLinkViconDataStreamSourceEditor::CreateSource ) ] ] ];
}

FReply SLiveLinkViconDataStreamSourceEditor::CreateSource() const
{
  ViconStreamProperties Properties;
  Properties.m_ServerName = GetServerName();
  Properties.m_SubjectFilter = GetSubjectFilter();
  Properties.m_PortNumber = GetPortNumber();
  Properties.m_bRetimed = GetIsRetimed();
  Properties.m_bLogOutput = GetLogOutput();
  Properties.m_bUsePrefetch = GetUsePrefetch();
  Properties.m_RetimeOffset = GetOffset();

  Properties.m_bScaled = true;

  OnPropertiesSelected.ExecuteIfBound( Properties );

  return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
