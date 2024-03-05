// Copyright (c) 2021 Vicon Motion Systems Ltd. All Rights Reserved.


#include "LiveLinkViconDataStreamSourceFactory.h"
#include "ILiveLinkDataStreamModule.h"
#include "LiveLinkViconDataStreamSource.h"
#include "LiveLinkViconDataStreamBlueprint.h"
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
  const FText SourceType = FText::FromString(FString(ULiveLinkViconDataStreamBlueprint::SOURCE_TYPE.c_str()));
  TSharedPtr< FLiveLinkViconDataStreamSource > NewSource = MakeShareable( new FLiveLinkViconDataStreamSource( SourceType, Props ) );
  return NewSource;
}

void ULiveLinkViconDataStreamSourceFactory::OnPropertiesSelected( ViconStreamProperties StreamProperties, FOnLiveLinkSourceCreated OnLiveLinkSourceCreated ) const
{
  FString PropertiesString = StreamProperties.ToString();
  const FText SourceType = FText::FromString(FString(ULiveLinkViconDataStreamBlueprint::SOURCE_TYPE.c_str()));
  TSharedPtr< FLiveLinkViconDataStreamSource > SharedPtr = MakeShared< FLiveLinkViconDataStreamSource >( SourceType, StreamProperties );
  OnLiveLinkSourceCreated.ExecuteIfBound( SharedPtr, MoveTemp( PropertiesString ) );
}
#undef LOCTEXT_NAMESPACE
