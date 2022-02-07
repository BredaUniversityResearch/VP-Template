#pragma once

#include "BaseLight.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"

#include "VirtualLight.generated.h"

// Extension of UBaseLight which represents a Virtual Light.
// To be used in the virtual light tool.
// Functions are overriden to update the values of the light actor which the object represents

UCLASS(BlueprintType)
class CRADLELIGHTCONTROL_API UVirtualLight : public UBaseLight
{
    GENERATED_BODY()

public:

    UVirtualLight()
        : UBaseLight() {};

    float GetIntensityNormalized() const;
    float GetTemperatureNormalized() const;

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
        class AActor* ActorPtr; // Generic actor pointer for when we don't care about the light type
        class ASkyLight* SkyLight;
        class APointLight* PointLight;
        class ADirectionalLight* DirectionalLight;
        class ASpotLight* SpotLight;
    };

    class AActor* OriginalActor;

    UPROPERTY()
        bool bCastShadows;
};
