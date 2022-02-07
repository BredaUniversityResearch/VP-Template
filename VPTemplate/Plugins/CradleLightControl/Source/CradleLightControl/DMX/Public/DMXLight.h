#pragma once


#include "DMXConfigAsset.h"

#include "BaseLight.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "IO/DMXOutputPort.h"

#include "DMXLight.generated.h"


UCLASS(BlueprintType)
class UDMXLight : public UBaseLight
{
public:
    GENERATED_BODY()
    UDMXLight()
        : StartingChannel(1)
        , bDMXEnabled(true)
        , Config(nullptr)
    {};

    virtual float GetHorizontalNormalized() const override;
    virtual float GetVerticalNormalized() const override;

    virtual void SetEnabled(bool bNewState) override;

    virtual void SetLightIntensity(float NormalizedValue) override;
    virtual void SetHue(float NewValue) override;
    virtual void SetSaturation(float NewValue) override;

    virtual void AddHorizontal(float NormalizedDegrees) override;
    virtual void AddVertical(float NormalizedDegrees) override;

    virtual FPlatformTypes::uint8 LoadFromJson(TSharedPtr<FJsonObject> JsonObject) override;
    virtual TSharedPtr<FJsonObject> SaveAsJson() override;

    virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;

    void UpdateDMXChannels();

    UPROPERTY(EditAnywhere)
        uint32 StartingChannel;

    UPROPERTY(EditAnywhere)
        bool bDMXEnabled;

    UPROPERTY(EditAnywhere)
        UDMXConfigAsset* Config;

    TSharedPtr<FDMXOutputPort, ESPMode::ThreadSafe> OutputPort;
    
};