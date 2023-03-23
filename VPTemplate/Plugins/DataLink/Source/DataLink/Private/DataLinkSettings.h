#pragma once
#include "Engine/DeveloperSettings.h"

#include "DataLinkSettings.generated.h"

USTRUCT()
struct FEndpointSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool ConnectOnStartup;

    UPROPERTY(EditAnywhere)
    FString Hostname {"127.0.0.1"};

    UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax=65535))
    int32 Port {1234};

};


UCLASS(config=Game, meta = (DisplayName="DataLink Settings"))
class UDataLinkSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:

    UDataLinkSettings() = default;

    UPROPERTY(Config, EditAnywhere)
        FEndpointSettings RemoteEndpointSettings;

    //The max amount of messages that can be buffered to be sent on (re-)connection.
    //Set to 0 to define infinite scaling queue.
    UPROPERTY(Config, EditAnywhere)
    int32 MaxMessageQueueSize {1024};
};
