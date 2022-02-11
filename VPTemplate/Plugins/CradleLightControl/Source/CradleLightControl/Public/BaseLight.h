#pragma once
#include "LightControlLoadingResult.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"


#include "BaseLight.generated.h"

// Base class representing a light in the tools' data.
// Contains data that is common between virtual and DMX controlled lights, and interface functions for them

UENUM()
enum ELightType
{
    Mixed = 0,
    SkyLight,
    SpotLight,
    DirectionalLight,
    PointLight,
    Invalid
};

UCLASS(BlueprintType)
class UBaseLight : public UObject
{
    GENERATED_BODY()

public:
    UBaseLight()
        : Intensity(0.0f)
        , Hue(0.0f)
        , Saturation(0.0f)
        , Temperature(0.0f)
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

    UFUNCTION(BlueprintCallable)
    virtual void SetEnabled(bool bNewState);
    UFUNCTION(BlueprintCallable)
        virtual void SetLightIntensity(float NormalizedValue);
    UFUNCTION(BlueprintCallable)
        virtual void SetLightIntensityRaw(float Value);
    UFUNCTION(BlueprintCallable)
        virtual void SetHue(float NewValue);
    UFUNCTION(BlueprintCallable)
        virtual void SetSaturation(float NewValue);
    UFUNCTION(BlueprintCallable)
        virtual void SetUseTemperature(bool NewState);
    UFUNCTION(BlueprintCallable)
        virtual void SetTemperature(float NormalizedValue);
    UFUNCTION(BlueprintCallable)
        virtual void SetTemperatureRaw(float Value);


    UFUNCTION(BlueprintCallable)
        virtual void SetCastShadows(bool bState);


    // Horizontal and vertical rotation are done using addition to preserve
    // relative rotation between lights when transforming multiple at a time


    UFUNCTION(BlueprintCallable)
        virtual void AddHorizontal(float NormalizedDegrees);
    UFUNCTION(BlueprintCallable)
        virtual void AddVertical(float NormalizedDegrees);
    UFUNCTION(BlueprintCallable)
        virtual void SetInnerConeAngle(float NewValue);
    UFUNCTION(BlueprintCallable)
        virtual void SetOuterConeAngle(float NewValue);

    // Returns a JSON object representative of the light
    virtual TSharedPtr<FJsonObject> SaveAsJson();
    // Loads light dta from the given JSON object
    virtual ::ELightControlLoadingResult LoadFromJson(TSharedPtr<FJsonObject> JsonObject);

    virtual void BeginTransaction();
#if WITH_EDITOR
    virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
#endif
    // Get the RGB color of the light based on its hue and saturation
    FLinearColor GetRGBColor() const;

    FString Name;
    ELightType Type;

    UPROPERTY()
        int32 Id;

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
    class UToolData* OwningToolData;
};
