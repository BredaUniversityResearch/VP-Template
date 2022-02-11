#pragma once

#include "Slate.h"

enum class EDataSet : uint8;
class UToolData;

class SLightPropertyEditor : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLightPropertyEditor)
		: _DisplayIntensityInPercentage(false)
	    , _DisplayTemperatureInPercentage(false)
	{}

    SLATE_ARGUMENT(class UEditorData*, EditorData)

    SLATE_ARGUMENT(bool, DisplayIntensityInPercentage)

    SLATE_ARGUMENT(bool, DisplayTemperatureInPercentage)

    SLATE_END_ARGS();

    static TArray<FColor> LinearGradient(TArray<FColor> ControlPoints, FVector2D Size = FVector2D(1.0f, 256.0f), EOrientation Orientation = Orient_Vertical);

    static UTexture2D* MakeGradientTexture(int X = 1, int Y = 256);


    void Construct(const FArguments& Args, class UEditorData* InEditorData, EDataSet InDataSet);
    void PreDestroy();


    FReply OnGelPaletteButtonClicked();
    void OnGelSelectionChanged(const FLinearColor& NewHSVColor);


    void GenerateTextures();
    void UpdateSaturationGradient(float NewHue);
    const FSlateBrush* GetSaturationGradientBrush() const;

    void EndTransaction();

    void OnIntensityValueChanged(float Value);
    void IntensityTransactionBegin();
    FText GetIntensityValueText() const;
    float GetIntensityValue() const;
    FText GetIntensityPercentage() const;

    void OnHueValueChanged(float Value);
    void HueTransactionBegin();
    FText GetHueValueText() const;
    float GetHueValue() const;
    FText GetHuePercentage() const;

    void OnSaturationValueChanged(float Value);
    void SaturationTransactionBegin();
    FText GetSaturationValueText() const;
    float GetSaturationValue() const;

    void OnTemperatureValueChanged(float Value);
    void TemperatureTransactionBegin();
    bool TemperatureEnabled() const;
    void OnTemperatureCheckboxChecked(ECheckBoxState NewState);
    ECheckBoxState GetTemperatureCheckboxChecked() const;
    FText GetTemperatureValueText() const;
    float GetTemperatureValue() const;
    FText GetTemperaturePercentage() const;

    //class SLightControlTool* CoreToolPtr;
    UEditorData* EditorData;
    EDataSet DataSet;

    TSharedPtr<FSlateImageBrush> IntensityGradientBrush;
    TSharedPtr<FSlateImageBrush> HSVGradientBrush;
    TSharedPtr<FSlateImageBrush> DefaultSaturationGradientBrush;
    TSharedPtr<FSlateImageBrush> SaturationGradientBrush;
    TSharedPtr<FSlateImageBrush> TemperatureGradientBrush;

    bool bDisplayIntensityInPercentage;

    bool bDisplayTemperatureInPercentage;


    UPROPERTY()
        UTexture2D* IntensityGradientTexture;
    UPROPERTY()
        UTexture2D* HSVGradientTexture;
    UPROPERTY()
        UTexture2D* DefaultSaturationGradientTexture;
    UPROPERTY()
        UTexture2D* SaturationGradientTexture;
    UPROPERTY()
        UTexture2D* TemperatureGradientTexture;
};
