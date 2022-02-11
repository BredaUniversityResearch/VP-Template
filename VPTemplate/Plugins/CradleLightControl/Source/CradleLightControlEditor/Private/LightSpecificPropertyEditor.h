#pragma once

#include "Slate.h"

#include "LightControlDMX.h"

enum class EDataSet : uint8;

class SLightSpecificProperties : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLightSpecificProperties){};

    SLATE_ARGUMENT(class UEditorData*, EditorData)

    SLATE_END_ARGS();

    void Construct(const FArguments& Args, class UEditorData* InEditorData, EDataSet InDataSet);

    void UpdateToolState();

private:

    void ClearSlot();

    void OnCastShadowsStateChanged(ECheckBoxState NewState);
    ECheckBoxState CastShadowsState() const;

    void ConstructDirectionalLightProperties();
    void ConstructSpotLightProperties();
    void ConstructPointLightProperties();

    void EndTransaction();

    void OnHorizontalValueChanged(float NormalizedValue);
    void BeginHorizontalTransaction();
    FText GetHorizontalValueText() const;
    float GetHorizontalValue() const;
    FText GetHorizontalPercentage() const;

    void OnVerticalValueChanged(float NormalizedValue);
    void BeginVerticalTransaction();
    FText GetVerticalValueText() const;
    float GetVerticalValue() const;
    FText GetVerticalPercentage() const;

    void OnInnerAngleValueChanged(float NormalizedValue);
    void BeginInnerAngleTransaction();
    void OnInnerAngleLockedStateChanged(ECheckBoxState NewState);
    ECheckBoxState InnerAngleLockedState() const;
    FText GetInnerAngleValueText() const;
    float GetInnerAngleValue() const;
    FText GetInnerAnglePercentage() const;

    void OnOuterAngleValueChanged(float NormalizedValue);
    void BeginOuterAngleTransaction();
    FText GetOuterAngleValueText() const;
    float GetOuterAngleValue() const;
    FText GetOuterAnglePercentage() const;


    class UEditorData* EditorData;
    EDataSet DataSet;

    TSharedPtr<SLightControlDMX> PortSelectorTest;

    TSharedPtr<SBox> ToolSlot;


};
