#pragma once

#include "BaseLight.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"

#include "VirtualLight.generated.h"

UCLASS()
class UVirtualLight : public UBaseLight
{
    GENERATED_BODY()

public:

    UVirtualLight()
        : UBaseLight() {};

    float GetIntensityNormalized() const override;
    float GetTemperatureNormalized() const override;

    virtual float GetHorizontalNormalized() const override;
    virtual float GetVerticalNormalized() const override;

    virtual void SetEnabled(bool bNewState) override;
    virtual void SetLightIntensity(float NormalizedValue) override;
    virtual void SetLightIntensityRaw(float Value) override;
    virtual void SetHue(float NewValue) override;
    virtual void SetSaturation(float NewValue) override;
    virtual void SetUseTemperature(bool NewState) override;
    virtual void SetTemperature(float NormalizedValue) override;
    virtual void SetTemperatureRaw(float Value) override;

    virtual void SetCastShadows(bool bState) override;


    virtual void AddHorizontal(float NormalizedDegrees) override;
    virtual void AddVertical(float NormalizedDegrees) override;
    virtual void SetInnerConeAngle(float NewValue) override;
    virtual void SetOuterConeAngle(float NewValue) override;

    virtual bool GetCastShadows() const override;

    virtual TSharedPtr<FJsonObject> SaveAsJson() override;
    virtual FPlatformTypes::uint8 LoadFromJson(TSharedPtr<FJsonObject> JsonObject) override;

    virtual void BeginTransaction() override;

    union
    {
        class AActor* ActorPtr;
        class ASkyLight* SkyLight;
        class APointLight* PointLight;
        class ADirectionalLight* DirectionalLight;
        class ASpotLight* SpotLight;
    };

    UPROPERTY()
        bool bCastShadows;
};
