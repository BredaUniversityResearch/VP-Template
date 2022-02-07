#include "DMXControlTool.h"

#include "Slate.h"

#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"

#include "ToolData.h"
#include "ItemHandle.h"
#include "BaseLight.h"
#include "DMXLight.h"

#include "CradleLightControlEditor.h"
#include "EditorData.h"
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"

#include "VirtualLight.h"
#include "Engine/AssetManager.h"

#define LOCTEXT_NAMESPACE "SDMXControlTool"

void SDMXControlTool::Construct(const FArguments& Args, UToolData* ToolData)
{
    SLightEditorWidget::Construct(Args, ToolData);    

    DataAutoSaveTimer = RegisterActiveTimer(300.0f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
        {
            EditorData->AutoSave();

            return EActiveTimerReturnType::Continue;
        }));

    EditorData->AddToRoot();
    
    auto& AddLightButtonSlot = HierarchyVerticalBox->AddSlot();
    AddLightButtonSlot
	[
        SNew(SButton)
        .Text(FText::FromString("New Light"))
        .OnClicked(this, &SDMXControlTool::AddLightButtonCallback)
    ];

    AddLightButtonSlot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


}

SDMXControlTool::~SDMXControlTool()
{
}

void SDMXControlTool::UpdateExtraLightDetailBox()
{
    if (EditorData->IsAMasterLightSelected())
    {
        ExtraLightDetailBox->SetContent(DMXChannelProperties());
    }
    else
        ExtraLightDetailBox->SetContent(SNew(SBox));

}

FReply SDMXControlTool::AddLightButtonCallback()
{
    auto NewItemHandle = EditorData->GetToolData()->AddItem();
    NewItemHandle->Name = "New DMX Light";
    NewItemHandle->Type = SpotLight;
    if (EditorData->IsSingleGroupSelected())
        EditorData->GetSelectedGroup()->Children.Add(NewItemHandle);
    else
        EditorData->GetToolData()->RootItems.Add(NewItemHandle);
	TreeWidget->GenerateWidgetForItem(NewItemHandle);
    EditorData->TreeStructureChangedDelegate.ExecuteIfBound();

    return FReply::Handled();
}



TSharedRef<SBox> SDMXControlTool::DMXChannelProperties()
{
    TSharedPtr<SBox> Box;


    auto MasterLightHandle = EditorData->GetMasterLight();
    auto MasterDMXLight = Cast<UDMXLight>(MasterLightHandle->Item);
    SAssignNew(Box, SBox)
        [
            SNew(SBorder)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
				.AutoHeight()
                [
                    SAssignNew(DMXPortSelector, SDMXPortSelector)
                    .Mode(EDMXPortSelectorMode::SelectFromAvailableInputsAndOutputs)
                    .OnPortSelected(this, &SDMXControlTool::OnPortSelected)
                ]
                +SVerticalBox::Slot()
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Use DMX"))
                    ]
                    +SHorizontalBox::Slot()
                    [
                        SNew(SCheckBox)
                        .IsChecked(this, &SDMXControlTool::UseDMXCheckboxState)
                        .OnCheckStateChanged(this, &SDMXControlTool::UseDMXCheckboxStateChanged)
                    ]
                ]
                +SVerticalBox::Slot()
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Channel Config"))
                    ]
                    +SHorizontalBox::Slot()
                    [
                        SNew(SObjectPropertyEntryBox)
                        .AllowedClass(UDMXConfigAsset::StaticClass())
                        .OnObjectChanged(this, &SDMXControlTool::OnSetDMXConfigAsset)
                        .ObjectPath(this, &SDMXControlTool::DMXConfigObjectPath)
                    ]
                ]
                +SVerticalBox::Slot()
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Starting Channel"))
                    ]
                    +SHorizontalBox::Slot()
                    [
                        SNew(SNumericEntryBox<int>)
                        .Value(this, &SDMXControlTool::StartingChannelBoxGetValue)
                        .OnValueCommitted(this, &SDMXControlTool::StartingChannelBoxValueCommitted)
                    ]
                ]
            ]
        ];
    if (MasterDMXLight->OutputPort)
    {
        DMXPortSelector->SelectPort(MasterDMXLight->OutputPort->GetPortGuid());
    }
    else
        DMXPortSelector->SelectPort(FGuid());
    if (DMXPortSelector->IsOutputPortSelected())
        MasterDMXLight->OutputPort = DMXPortSelector->GetSelectedOutputPort();

    return Box.ToSharedRef();
}

void SDMXControlTool::OnPortSelected()
{

    auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);
    auto OutputPort = DMXPortSelector->GetSelectedOutputPort();
    if (OutputPort)
    {
		GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX port changed"));
		MasterDMX->BeginTransaction();
        MasterDMX->OutputPort = OutputPort;
        MasterDMX->UpdateDMXChannels();
        GEditor->EndTransaction();
    }
}

ECheckBoxState SDMXControlTool::UseDMXCheckboxState() const
{
    auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);
    return MasterDMX->bDMXEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SDMXControlTool::UseDMXCheckboxStateChanged(ECheckBoxState NewState)
{
    auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);

    GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX usage changed"));
    MasterDMX->BeginTransaction();
    MasterDMX->bDMXEnabled = NewState == ECheckBoxState::Checked;
    MasterDMX->UpdateDMXChannels();
    GEditor->EndTransaction();
}

FString SDMXControlTool::DMXConfigObjectPath() const
{
    auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);

    FString ObjectPath = "";
    if (MasterDMX->Config)
    {
        ObjectPath = MasterDMX->Config->GetAssetPath();
    }
    return ObjectPath;
}

void SDMXControlTool::OnSetDMXConfigAsset(const FAssetData& AssetData)
{
    auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);
    GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX config changed"));
    MasterDMX->BeginTransaction();
    MasterDMX->Config = DuplicateObject(Cast<UDMXConfigAsset>(AssetData.GetAsset()), MasterDMX);
    MasterDMX->Config->AssetName = AssetData.AssetName;
    //MasterDMX->Config
}

TOptional<int> SDMXControlTool::StartingChannelBoxGetValue() const
{
    auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);
    return MasterDMX->StartingChannel;
}

void SDMXControlTool::StartingChannelBoxValueCommitted(int Value, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::Default)
    {
        auto MasterDMX = Cast<UDMXLight>(EditorData->GetMasterLight()->Item);
        GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX starting channel changed"));
        MasterDMX->BeginTransaction();
        MasterDMX->StartingChannel = Value;
        MasterDMX->UpdateDMXChannels();
        GEditor->EndTransaction();
    }
}

#undef LOCTEXT_NAMESPACE