#pragma once
#include "Engine/DeveloperSettings.h"
#include "DataLinkSettings.generated.h"

UCLASS(config=Game, meta = (DisplayName="DataLink Settings"))
class DATALINK_API UDataLinkSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:

    UDataLinkSettings() = default;

    UPROPERTY(Config, EditAnywhere)
    FString RemoteEndpoint;
};
