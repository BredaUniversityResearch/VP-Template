#pragma once
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"


#include "BaseLight.generated.h"

UCLASS()
class UBaseLight : public UObject
{
    GENERATED_BODY()

public:
    UBaseLight()
        : Intensity(0.0f)
        , Temperature(0.0f)
        , Hue(0.0f)
        , Saturation(0.0f)
        , Horizontal(0.0f)
        , Vertical(0.0f)
        , InnerAngle(0.0f)
        , OuterAngle(0.0f)
    {
        SetFlags(GetFlags() | RF_Transactional);
    };

    bool IsEnabled() const { return bIsEnabled; };

    virtual float GetIntensityNormalized() const { return Intensity; }
    virtual float GetHue() const { return Hue; };
    virtual float GetSaturation() const { return Saturation; };
    virtual bool GetUseTemperature() const { return bUseTemperature; };
    virtual float GetTemperatureNormalized() const { return Temperature; };

    virtual bool GetCastShadows() const { return false; };

    virtual float GetHorizontalNormalized() const { return Horizontal; };
    virtual float GetVerticalNormalized() const { return Vertical; };
    virtual float GetInnerConeAngle() const { return InnerAngle; };
    virtual float GetOuterConeAngle() const { return OuterAngle; };

    virtual void SetEnabled(bool bNewState);
    virtual void SetLightIntensity(float NormalizedValue);
    virtual void SetLightIntensityRaw(float Value);
    virtual void SetHue(float NewValue);
    virtual void SetSaturation(float NewValue);
    virtual void SetUseTemperature(bool NewState);
    virtual void SetTemperature(float NormalizedValue);
    virtual void SetTemperatureRaw(float Value);


    virtual void SetCastShadows(bool bState);

    virtual void AddHorizontal(float NormalizedDegrees);
    virtual void AddVertical(float NormalizedDegrees);
    virtual void SetInnerConeAngle(float NewValue);
    virtual void SetOuterConeAngle(float NewValue);

    virtual TSharedPtr<FJsonObject> SaveAsJson();
    virtual FPlatformTypes::uint8 LoadFromJson(TSharedPtr<FJsonObject> JsonObject);

    virtual void BeginTransaction();
    virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;

    FLinearColor GetRGBColor() const;




    UPROPERTY(BlueprintReadOnly)
        bool bIsEnabled;

    UPROPERTY(BlueprintReadOnly)
        float Intensity;
    UPROPERTY(BlueprintReadOnly)
        float Hue;
    UPROPERTY(BlueprintReadOnly)
        float Saturation;
    UPROPERTY(BlueprintReadOnly)
        bool bUseTemperature;
    UPROPERTY(BlueprintReadOnly)
        float Temperature;

    UPROPERTY(BlueprintReadOnly)
        float Horizontal;
    UPROPERTY(BlueprintReadOnly)
        float Vertical;
    UPROPERTY(BlueprintReadOnly)
        float InnerAngle;
    UPROPERTY(BlueprintReadOnly)
        float OuterAngle;
    UPROPERTY()
        bool bLockInnerAngleToOuterAngle;

    // Not UPROPERTY to avoid circular reference with what is essentially shared pointers
    class UItemHandle* Handle;

};
