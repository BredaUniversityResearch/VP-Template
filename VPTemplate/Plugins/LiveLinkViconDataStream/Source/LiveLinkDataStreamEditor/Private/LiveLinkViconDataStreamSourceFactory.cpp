// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.


#include "LiveLinkViconDataStreamSourceFactory.h"
#include "ILiveLinkDataStreamModule.h"
#include "LiveLinkViconDataStreamSource.h"
#include "LiveLinkViconDataStreamSourceEditor.h"

#define LOCTEXT_NAMESPACE "LiveLinkViconDataStreamSourceFactory"

FText ULiveLinkViconDataStreamSourceFactory::GetSourceDisplayName() const
{
  return LOCTEXT( "SourceDisplayName", "Vicon Data Stream Source" );
}

FText ULiveLinkViconDataStreamSourceFactory::GetSourceTooltip() const
{
  return LOCTEXT( "SourceTooltip", "Creates a connection to a Vicon Data Stream based Live Link Source" );
}

TSharedPtr< SWidget > ULiveLinkViconDataStreamSourceFactory::BuildCreationPanel( FOnLiveLinkSourceCreated OnLiveLinkSourceCreated ) const
{
  return SNew( SLiveLinkViconDataStreamSourceEditor )
    .OnPropertiesSelected( FOnDataStreamPropertiesSelected::CreateUObject( this, &ULiveLinkViconDataStreamSourceFactory::OnPropertiesSelected, OnLiveLinkSourceCreated ) );
}

/** Create a new source from a ConnectionString */
TSharedPtr< ILiveLinkSource > ULiveLinkViconDataStreamSourceFactory::CreateSource( const FString& ConnectionString ) const
{
  // Extract the properties from the string
  ViconStreamProperties Props = ViconStreamProperties::FromString( ConnectionString );
  TSharedPtr< FLiveLinkViconDataStreamSource > NewSource = MakeShareable( new FLiveLinkViconDataStreamSource( FText::FromString( "Vicon Live Link" ), Props ) );
  return NewSource;
}

void ULiveLinkViconDataStreamSourceFactory::OnPropertiesSelected( ViconStreamProperties StreamProperties, FOnLiveLinkSourceCreated OnLiveLinkSourceCreated ) const
{
  FString PropertiesString = StreamProperties.ToString();
  TSharedPtr< FLiveLinkViconDataStreamSource > SharedPtr = MakeShared< FLiveLinkViconDataStreamSource >( FText::FromString( "Vicon Live Link" ), StreamProperties );
  OnLiveLinkSourceCreated.ExecuteIfBound( SharedPtr, MoveTemp( PropertiesString ) );
}
#undef LOCTEXT_NAMESPACE
