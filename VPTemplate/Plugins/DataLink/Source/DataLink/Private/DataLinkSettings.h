#pragma once
#include "Engine/DeveloperSettings.h"

UCLASS(config=Game, meta = (DisplayName="DataLink Settings"))
class UDataLinkSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    FString RemoteEndpoint;

};
