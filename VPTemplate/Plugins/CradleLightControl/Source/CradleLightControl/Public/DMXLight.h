#pragma once


#include "DMXConfigAsset.h"

#include "BaseLight.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "IO/DMXOutputPort.h"

#include "DMXLight.generated.h"

// Extension of UBaseLight which represents a DMX light fixture.
// Meant to be used with the DMX control tool and its UToolData
// Overrides setter functions to also update the relevant DMX channels based on the set starting channel and DMXConfig asset

UCLASS(BlueprintType)
class CRADLELIGHTCONTROL_API UDMXLight : public UBaseLight
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

    virtual ::ELightControlLoadingResult LoadFromJson(TSharedPtr<FJsonObject> JsonObject) override;
    virtual TSharedPtr<FJsonObject> SaveAsJson() override;
#if WITH_EDITOR
    virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
#endif
    void UpdateDMXChannels();

    // The first channel to be affected by this DMX fixture. NOT 0-based.
    // Starting channel 1 will correlate to the first channel in the protocol. 
    UPROPERTY(EditAnywhere)
        uint32 StartingChannel;

    // Is this fixture enabled to send DMX signals?
    UPROPERTY(EditAnywhere)
        bool bDMXEnabled;

    // Reference to the config asset to use with this fixture.
    UPROPERTY(EditAnywhere)
        UDMXConfigAsset* Config;

    // The output port by which to the DMX signal can reach the physical DMX fixture.
    TSharedPtr<FDMXOutputPort, ESPMode::ThreadSafe> OutputPort;

};