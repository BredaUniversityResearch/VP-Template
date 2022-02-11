#include "LightSpecificPropertyEditor.h"
#include "LightControlTool.h"

#include "EditorData.h"
#include "Itemhandle.h"
#include "BaseLight.h"
#include "LightRuntimeNetwork.h"
#include "LightEditorNetwork.h"
#include "CradleLightControlEditor.h"

#include "Engine/SpotLight.h"

void SLightSpecificProperties::Construct(const FArguments& Args, class UEditorData* InEditorData, EDataSet InDataSet)
{
    _ASSERT(Args._ToolData);
    EditorData = InEditorData;
    DataSet = InDataSet;

    ChildSlot
    [
        SNew(SBorder)
        [
            SAssignNew(ToolSlot, SBox)
        ]
    ];

    UpdateToolState();
}

void SLightSpecificProperties::UpdateToolState()
{
    if (!EditorData->IsAMasterLightSelected())
    {
        ClearSlot();
        return;
    }

    auto LightHandle = EditorData->GetMasterLight();

    if (LightHandle->Item->Type == SpotLight)
    {
        ConstructSpotLightProperties();
    }
    else if (LightHandle->Item->Type == DirectionalLight)
    {
        ConstructDirectionalLightProperties();
    }
    else if (LightHandle->Item->Type == PointLight || LightHandle->Item->Type == SkyLight)
    {
        ConstructPointLightProperties();
    }
    else
        ClearSlot();
}

void SLightSpecificProperties::ClearSlot()
{
    ToolSlot->SetVisibility(EVisibility::Hidden);

    ToolSlot->SetContent(
        SNew(SBox)
        //SAssignNew(PortSelectorTest, SLightControlDMX)
        //.CoreToolPtr(CoreToolPtr)
    );
}

void SLightSpecificProperties::OnCastShadowsStateChanged(ECheckBoxState NewState)
{
    GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " Cast Shadows"));
    for (auto Light : EditorData->LightsUnderSelection)
    {
        Light->BeginTransaction();
        Light->Item->SetCastShadows(NewState == ECheckBoxState::Checked);
    }
    EndTransaction();
}

ECheckBoxState SLightSpecificProperties::CastShadowsState() const
{
    if (EditorData->IsAMasterLightSelected())
    {
        auto MasterLight = EditorData->GetMasterLight();

        auto MasterLightCastShadows = MasterLight->Item->GetCastShadows();
        auto State = MasterLightCastShadows ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        for (auto Light : EditorData->LightsUnderSelection)
        {
            if (Light->Item->GetCastShadows() != MasterLightCastShadows)
            {
                State = ECheckBoxState::Undetermined;
                break;
            }
        }

        return State;
    }
    return ECheckBoxState::Undetermined;
}

void SLightSpecificProperties::ConstructDirectionalLightProperties()
{
    SVerticalBox::FSlot* HorizontalNameSlot, * HorizontalDegreesSlot, * HorizontalPercentageSlot;
    SVerticalBox::FSlot* VerticalNameSlot, * VerticalDegreesSlot, * VerticalPercentageSlot;
    SVerticalBox::FSlot* CastShadowsSlot;
    SHorizontalBox::FSlot* CastShadowsNameSlot;

    ToolSlot->SetVisibility(EVisibility::Visible);
    ToolSlot->SetContent(
        SNew(SVerticalBox)
            +SVerticalBox::Slot()
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .Expose(HorizontalNameSlot)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Horizontal"))
                    ]
                    +SVerticalBox::Slot()
                    .Expose(HorizontalDegreesSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetHorizontalValueText)
                    ]
                    +SVerticalBox::Slot()
                    [
                        SNew(SSlider)
                        .Orientation(Orient_Vertical)
                        .OnValueChanged(this, &SLightSpecificProperties::OnHorizontalValueChanged)
                        .Value(this, &SLightSpecificProperties::GetHorizontalValue)
                        .OnMouseCaptureBegin(this, &SLightSpecificProperties::BeginHorizontalTransaction)
                        .OnMouseCaptureEnd(this, &SLightSpecificProperties::EndTransaction)
                    ]
                    + SVerticalBox::Slot()
                    .Expose(HorizontalPercentageSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetHorizontalPercentage)

                    ]
                ]
                +SHorizontalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .Expose(VerticalNameSlot)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Vertical"))
                    ]
                    +SVerticalBox::Slot()
                    .Expose(VerticalDegreesSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetVerticalValueText)
                    ]
                    +SVerticalBox::Slot()
                    [
                        SNew(SSlider)
                        .Orientation(Orient_Vertical)
                        .OnValueChanged(this, &SLightSpecificProperties::OnVerticalValueChanged)
                        .Value(this, &SLightSpecificProperties::GetVerticalValue)
                        .OnMouseCaptureBegin(this, &SLightSpecificProperties::BeginVerticalTransaction)
                        .OnMouseCaptureEnd(this, &SLightSpecificProperties::EndTransaction)
                    ]
                    + SVerticalBox::Slot()
                    .Expose(VerticalPercentageSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetVerticalPercentage)
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .Expose(CastShadowsSlot)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .Expose(CastShadowsNameSlot)
                .Padding(10.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Cast Shadows"))
                ]
                + SHorizontalBox::Slot()
                [
                    SNew(SCheckBox)
                    .OnCheckStateChanged(this, &SLightSpecificProperties::OnCastShadowsStateChanged)
                    .IsChecked(this, &SLightSpecificProperties::CastShadowsState)
                ]
            ]);


    HorizontalNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    HorizontalDegreesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    HorizontalPercentageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    VerticalNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    VerticalDegreesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    VerticalPercentageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    CastShadowsSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    CastShadowsNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
}

void SLightSpecificProperties::ConstructSpotLightProperties()
{
    SVerticalBox::FSlot* HorizontalNameSlot, *HorizontalDegreesSlot, *HorizontalPercentageSlot;
    SVerticalBox::FSlot* VerticalNameSlot, *VerticalDegreesSlot, *VerticalPercentageSlot;
    SVerticalBox::FSlot* OuterAngleNameSlot, *OuterAngleDegreesSlot, *OuterAnglePercentageSlot;
    SVerticalBox::FSlot* InnerAngleNameSlot, *InnerAngleCheckboxSlot, *InnerAngleDegreesSlot, *InnerAnglePercentageSlot;
    SVerticalBox::FSlot* CastShadowsSlot;
    //SVerticalBox::FSlot* DMXSlot;
    SHorizontalBox::FSlot* CastShadowsNameSlot;
    ToolSlot->SetVisibility(EVisibility::Visible);
    ToolSlot->SetContent(
        SNew(SVerticalBox)
            +SVerticalBox::Slot()
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .Expose(HorizontalNameSlot)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Horizontal"))
                    ]
                    +SVerticalBox::Slot()
                    .Expose(HorizontalDegreesSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetHorizontalValueText)
                    ]
                    +SVerticalBox::Slot()
                    [
                        SNew(SSlider)
                        .Orientation(Orient_Vertical)
                        .OnValueChanged(this, &SLightSpecificProperties::OnHorizontalValueChanged)
                        .Value(this, &SLightSpecificProperties::GetHorizontalValue)
                        .OnMouseCaptureBegin(this, &SLightSpecificProperties::BeginHorizontalTransaction)
                        .OnMouseCaptureEnd(this, &SLightSpecificProperties::EndTransaction)
                    ]
                    + SVerticalBox::Slot()
                    .Expose(HorizontalPercentageSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetHorizontalPercentage)
                    ]
                ]
                +SHorizontalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .Expose(VerticalNameSlot)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Vertical"))
                    ]
                    +SVerticalBox::Slot()
                    .Expose(VerticalDegreesSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetVerticalValueText)
                    ]
                    +SVerticalBox::Slot()
                    [
                        SNew(SSlider)
                        .Orientation(Orient_Vertical)
                        .OnValueChanged(this, &SLightSpecificProperties::OnVerticalValueChanged)
                        .Value(this, &SLightSpecificProperties::GetVerticalValue)
                        .OnMouseCaptureBegin(this, &SLightSpecificProperties::BeginVerticalTransaction)
                        .OnMouseCaptureEnd(this, &SLightSpecificProperties::EndTransaction)
                    ]
                    + SVerticalBox::Slot()
                    .Expose(VerticalPercentageSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetVerticalPercentage)
                    ]
                ]
                +SHorizontalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .Expose(OuterAngleNameSlot)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Outer Angle"))
                    ]
                    +SVerticalBox::Slot()
                    .Expose(OuterAngleDegreesSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetOuterAngleValueText)
                    ]
                    +SVerticalBox::Slot()
                    [
                        SNew(SSlider)
                        .Orientation(Orient_Vertical)
                        .OnValueChanged(this, &SLightSpecificProperties::OnOuterAngleValueChanged)
                        .Value(this, &SLightSpecificProperties::GetOuterAngleValue)
                        .OnMouseCaptureBegin(this, &SLightSpecificProperties::BeginOuterAngleTransaction)
                        .OnMouseCaptureEnd(this, &SLightSpecificProperties::EndTransaction)
                    ]
                    + SVerticalBox::Slot()
                    .Expose(OuterAnglePercentageSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetOuterAnglePercentage)
                    ]
                ]
                + SHorizontalBox::Slot()
                    .Padding(5.0f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                    .Expose(InnerAngleNameSlot)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Inner Angle"))
                    ]
                + SVerticalBox::Slot()
                    .Expose(InnerAngleDegreesSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetInnerAngleValueText)
                    ]
                + SVerticalBox::Slot()
                    .Expose(InnerAngleCheckboxSlot)
                    [
                        SNew(SCheckBox)
                        .ToolTipText(FText::FromString("Should the inner angle change proportionally to the outer angle?"))
                        .IsChecked(this, &SLightSpecificProperties::InnerAngleLockedState)
                        .OnCheckStateChanged(this, &SLightSpecificProperties::OnInnerAngleLockedStateChanged)
                    ]
                + SVerticalBox::Slot()
                    [
                        SNew(SSlider)
                        .Orientation(Orient_Vertical)
                        .OnValueChanged(this, &SLightSpecificProperties::OnInnerAngleValueChanged)
                        .Value(this, &SLightSpecificProperties::GetInnerAngleValue)
                        .OnMouseCaptureBegin(this, &SLightSpecificProperties::BeginInnerAngleTransaction)
                        .OnMouseCaptureEnd(this, &SLightSpecificProperties::EndTransaction)
                    ]
                + SVerticalBox::Slot()
                    .Expose(InnerAnglePercentageSlot)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightSpecificProperties::GetInnerAnglePercentage)
                    ]
                    ]
            ]
            +SVerticalBox::Slot()
            .Expose(CastShadowsSlot)
            [
                SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .Expose(CastShadowsNameSlot)
                .Padding(10.0f)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Cast Shadows"))
                ]
                +SHorizontalBox::Slot()
                [
                    SNew(SCheckBox)
                    .OnCheckStateChanged(this, &SLightSpecificProperties::OnCastShadowsStateChanged)
                    .IsChecked(this, &SLightSpecificProperties::CastShadowsState)
                ]
            ]
            //+SVerticalBox::Slot()
            //.Expose(DMXSlot)
            //[
            //    SNew(SLightControlDMX)
            //    //.CoreToolPtr(CoreToolPtr)
            //]

    );

    HorizontalNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    HorizontalDegreesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    HorizontalPercentageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    VerticalNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    VerticalDegreesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    VerticalPercentageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    OuterAngleNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    OuterAngleDegreesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    OuterAnglePercentageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    InnerAngleNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    InnerAngleDegreesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    InnerAngleCheckboxSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    InnerAnglePercentageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    //DMXSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    CastShadowsSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    CastShadowsNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
}

void SLightSpecificProperties::ConstructPointLightProperties()
{
    SVerticalBox::FSlot* CastShadowsSlot;
    SHorizontalBox::FSlot* CastShadowsNameSlot;
    ToolSlot->SetVisibility(EVisibility::Visible);
    ToolSlot->SetContent(
        SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .Expose(CastShadowsSlot)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Expose(CastShadowsNameSlot)
            .Padding(10.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString("Cast Shadows"))
            ]
            + SHorizontalBox::Slot()
            [
                SNew(SCheckBox)
                .OnCheckStateChanged(this, &SLightSpecificProperties::OnCastShadowsStateChanged)
                .IsChecked(this, &SLightSpecificProperties::CastShadowsState)
            ]
        ]);
        

    CastShadowsSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    CastShadowsNameSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
}

void SLightSpecificProperties::EndTransaction()
{
    GEditor->EndTransaction();
}

void SLightSpecificProperties::OnHorizontalValueChanged(float NormalizedValue)
{
    auto Light = EditorData->GetMasterLight();
    auto Delta = NormalizedValue - Light->Item->GetHorizontalNormalized();

    auto Lights = FCradleLightControlEditorModule::GetLightsFromHandles(EditorData->LightsUnderSelection);
	FCradleLightControlEditorModule::GetLightPropertyChangeSpeaker().SendLightPropertyChangeEvent(DataSet, Lights, EProperty::Horizontal, Delta);
        
}

void SLightSpecificProperties::BeginHorizontalTransaction()
{
    GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " Horizontal Rotation"));
}

FText SLightSpecificProperties::GetHorizontalValueText() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::Printf(TEXT("%.0f"), Light->Item->Horizontal));
}

float SLightSpecificProperties::GetHorizontalValue() const
{
    auto Light = EditorData->GetMasterLight();

    return Light->Item->GetHorizontalNormalized();
}

FText SLightSpecificProperties::GetHorizontalPercentage() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::FormatAsNumber(Light->Item->GetHorizontalNormalized() * 100.0f) + "%");
}

void SLightSpecificProperties::OnVerticalValueChanged(float NormalizedValue)
{
    auto Light = EditorData->GetMasterLight();
    auto Delta = NormalizedValue - Light->Item->GetVerticalNormalized();

    auto Lights = FCradleLightControlEditorModule::GetLightsFromHandles(EditorData->LightsUnderSelection);

	FCradleLightControlEditorModule::GetLightPropertyChangeSpeaker().SendLightPropertyChangeEvent(DataSet, Lights, EProperty::Vertical, Delta);
    
}

void SLightSpecificProperties::BeginVerticalTransaction()
{
    GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " Vertical Rotation"));

}


FText SLightSpecificProperties::GetVerticalValueText() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::Printf(TEXT("%.0f"), Light->Item->Vertical));
}

float SLightSpecificProperties::GetVerticalValue() const
{
    auto Light = EditorData->GetMasterLight();

    return Light->Item->GetVerticalNormalized();
}

FText SLightSpecificProperties::GetVerticalPercentage() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::FormatAsNumber(Light->Item->GetVerticalNormalized() * 100.0f) + "%");
}

void SLightSpecificProperties::OnInnerAngleValueChanged(float NormalizedValue)
{
    auto Light = EditorData->GetMasterLight();
    auto Angle = NormalizedValue * 80.0f;


    auto Lights = FCradleLightControlEditorModule::GetLightsFromHandles(EditorData->LightsUnderSelection);

    FCradleLightControlEditorModule::GetLightPropertyChangeSpeaker().SendLightPropertyChangeEvent(DataSet, Lights, EProperty::InnerConeAngle, Angle);
    
}

void SLightSpecificProperties::BeginInnerAngleTransaction()
{
    GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " Inner Angle"));
}

void SLightSpecificProperties::OnInnerAngleLockedStateChanged(ECheckBoxState NewState)
{
    if (EditorData)
    {
        GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " Inner Angle Lock"));
        for (auto Light : EditorData->LightsUnderSelection)
        {
            Light->BeginTransaction(true);
            Light->Item->bLockInnerAngleToOuterAngle = NewState == ECheckBoxState::Checked;
        }
        EndTransaction();
    }
}

ECheckBoxState SLightSpecificProperties::InnerAngleLockedState() const
{
    auto Light = EditorData->GetMasterLight();
    if (Light)
    {
        return Light->Item->bLockInnerAngleToOuterAngle ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }
    return ECheckBoxState::Undetermined;
}

FText SLightSpecificProperties::GetInnerAngleValueText() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::Printf(TEXT("%.0f"), Light->Item->InnerAngle));
}

float SLightSpecificProperties::GetInnerAngleValue() const
{
    auto Light = EditorData->GetMasterLight();

    return Light->Item->InnerAngle / 80.0f;
}

FText SLightSpecificProperties::GetInnerAnglePercentage() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::FormatAsNumber(Light->Item->InnerAngle / 0.8f) + "%");
}


void SLightSpecificProperties::OnOuterAngleValueChanged(float NormalizedValue)
{
    auto Light = EditorData->GetMasterLight();
    auto Angle = NormalizedValue * 80.0f;

    auto Lights = FCradleLightControlEditorModule::GetLightsFromHandles(EditorData->LightsUnderSelection);

	FCradleLightControlEditorModule::GetLightPropertyChangeSpeaker().SendLightPropertyChangeEvent(DataSet, Lights, EProperty::OuterConeAngle, Angle);
    
}

void SLightSpecificProperties::BeginOuterAngleTransaction()
{
    GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " Outer Angle"));
}

FText SLightSpecificProperties::GetOuterAngleValueText() const
{
    auto Light = EditorData->GetMasterLight();

    return FText::FromString(FString::Printf(TEXT("%.0f"), Light->Item->OuterAngle));
}

float SLightSpecificProperties::GetOuterAngleValue() const
{
    auto Light = EditorData->GetMasterLight();

    return Light->Item->OuterAngle / 80.0f;
}

FText SLightSpecificProperties::GetOuterAnglePercentage() const
{
    auto Light = EditorData->GetMasterLight();


    return FText::FromString(FString::FormatAsNumber(Light->Item->OuterAngle / 0.8f) + "%");
}
