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
#include "PropertyCustomizationHelpers.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"

#include "VirtualLight.h"
#include "Engine/AssetManager.h"

#define LOCTEXT_NAMESPACE "SDMXControlTool"

void SDMXControlTool::Construct(const FArguments& Args)
{
    ToolTab = Args._ToolTab;

    ToolData = NewObject<UToolData>();
    ToolData->DataName = "DMXLights";
    ToolData->ItemClass = UDMXLight::StaticClass();
    ToolData->OpenFileDialog = FLightJsonFileDialogDelegate::CreateRaw(this, &SDMXControlTool::OpenFileDialog);
    ToolData->SaveFileDialog = FLightJsonFileDialogDelegate::CreateRaw(this, &SDMXControlTool::SaveFileDialog);
    ToolData->MasterLightTransactedDelegate = FOnMasterLightTransactedDelegate::CreateLambda([this](UItemHandle* ItemHandle)
        {
            LightPropertyWidget->UpdateSaturationGradient(ItemHandle->Item->GetHue());
        });
    ToolData->LoadMetaData();

    DataAutoSaveTimer = RegisterActiveTimer(300.0f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
        {
            ToolData->AutoSave();

            return EActiveTimerReturnType::Continue;
        }));

    ToolData->AddToRoot();


    SVerticalBox::FSlot* AddLightButtonSlot;

    SSplitter::FSlot* SplitterSlot;
    ChildSlot
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Top)
            [
                SNew(SSplitter)
                .PhysicalSplitterHandleSize(5.0f)
                .HitDetectionSplitterHandleSize(15.0f)
                + SSplitter::Slot()
                .Expose(SplitterSlot)
                .Value(0.5f)
                [
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    [
                        SAssignNew(TreeWidget, SLightTreeHierarchy)
                        .ToolData(ToolData)
                        .Name("DMX Lights")
                        .SelectionChangedDelegate(FTreeSelectionChangedDelegate::CreateRaw(this, &SDMXControlTool::OnTreeSelectionChanged))
                    ]
                    +SVerticalBox::Slot()
                    .Expose(AddLightButtonSlot)
                    [
                        SNew(SButton)
                        .Text(FText::FromString("New Light"))
                        .OnClicked(this, &SDMXControlTool::AddLightButtonCallback)
                    ]
                ]
                + SSplitter::Slot()
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    [
                        SNew(SVerticalBox)
                        + LightHeader()
                        + LightPropertyEditor()
                    ]
                ]
            ]
        ];

    AddLightButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    
}

SDMXControlTool::~SDMXControlTool()
{
    //PreDestroy();
}

void SDMXControlTool::PreDestroy()
{
    ToolData->AutoSave();
    if (TreeWidget)
        TreeWidget->PreDestroy();
    if (LightPropertyWidget)
        LightPropertyWidget->PreDestroy();

    GWorld->RemoveOnActorSpawnedHandler(ActorSpawnedListenerHandle);
    UnRegisterActiveTimer(DataAutoSaveTimer.ToSharedRef());
}

void SDMXControlTool::OnTreeSelectionChanged()
{
    if (ToolData->IsAMasterLightSelected())
    {
        LightPropertyWidget->UpdateSaturationGradient(ToolData->SelectionMasterLight->Item->Hue);
        UpdateExtraLightDetailBox();
        ItemHeader->Update();
        LightSpecificWidget->UpdateToolState();
    }

}

TWeakPtr<SLightTreeHierarchy> SDMXControlTool::GetTreeWidget()
{
    return TreeWidget;
}

TWeakPtr<SLightPropertyEditor> SDMXControlTool::GetLightPropertyEditor()
{
    return LightPropertyWidget;
}

FString SDMXControlTool::OpenFileDialog(FString Title, FString StartingPath)
{

    TArray<FString> Res;
    if (FCradleLightControlEditorModule::OpenFileDialog(Title, ToolTab->GetParentWindow()->GetNativeWindow()->GetOSWindowHandle(),
        StartingPath, EFileDialogFlags::None, "JSON Data Table|*.json", Res))
    {
        return Res[0];
    }
    return "";
}

FString SDMXControlTool::SaveFileDialog(FString Title, FString StartingPath)
{
    TArray<FString> Res;
    if (FCradleLightControlEditorModule::SaveFileDialog(Title, ToolTab->GetParentWindow()->GetNativeWindow()->GetOSWindowHandle(),
        StartingPath, EFileDialogFlags::None, "JSON Data Table|*.json", Res))
    {
        return Res[0];
    }
    return "";
}

UToolData* SDMXControlTool::GetToolData() const
{
    return ToolData;
}

TSharedRef<SDockTab> SDMXControlTool::Show()
{
    if (!ToolTab)
    {
        ToolTab = SNew(SDockTab)
            .TabRole(NomadTab)
            .Label(LOCTEXT("DMXControlLabel", "DMX Light Control"))
            .OnTabClosed_Lambda([this](TSharedRef<SDockTab>)
                {
                    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("DMXControl");
                    ToolTab.Reset();
                })
            [
                SharedThis(this)
            ];
    }
    else
        ToolTab->FlashTab();

    return ToolTab.ToSharedRef();
}

void SDMXControlTool::LoadResources()
{

}

SVerticalBox::FSlot& SDMXControlTool::LightHeader()
{
    auto& Slot = SVerticalBox::Slot();

    Slot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    Slot
        .HAlign(HAlign_Fill)
        [
            SAssignNew(ItemHeader, SLightItemHeader)
            .ToolData(ToolData)
			.TreeHierarchyWidget(TreeWidget)
        ];

    //UpdateLightHeader();

    return Slot;

}

FReply SDMXControlTool::AddLightButtonCallback()
{
    auto NewItemHandle = ToolData->AddItem();
    NewItemHandle->Name = "New DMX Light";
    NewItemHandle->Type = SpotLight;
    if (ToolData->IsSingleGroupSelected())
        ToolData->GetSelectedGroup()->Children.Add(NewItemHandle);
    else
        ToolData->RootItems.Add(NewItemHandle);
	TreeWidget->GenerateWidgetForItem(NewItemHandle);
    ToolData->TreeStructureChangedDelegate.ExecuteIfBound();

    return FReply::Handled();
}


SVerticalBox::FSlot& SDMXControlTool::LightPropertyEditor()
{
    auto& Slot = SVerticalBox::Slot();

    TSharedPtr<SVerticalBox> Box;

    SVerticalBox::FSlot* ExtraLightBoxSlot;

    Slot
        .Padding(20.0f, 30.0f, 20.0f, 0.0f)
        .VAlign(VAlign_Fill)
        .HAlign(HAlign_Fill)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot() // General light properties + extra light properties or group controls
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        [
            SAssignNew(LightPropertyWidget, SLightPropertyEditor)
            .ToolData(ToolData)
			.DisplayIntensityInPercentage(true)
			.DisplayTemperatureInPercentage(true)
        ]
    + SVerticalBox::Slot()
        .Expose(ExtraLightBoxSlot)
        [
            SAssignNew(ExtraLightDetailBox, SBox)
            .Padding(FMargin(0.0f, 5.0f, 0.0f, 0.0f))
        ]
        ]
    + LightSpecificPropertyEditor()
        ];

    ExtraLightBoxSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    UpdateExtraLightDetailBox();

    return Slot;
}

void SDMXControlTool::UpdateExtraLightDetailBox()
{
    if (ToolData->IsAMasterLightSelected())
    {
        if (ToolData->MultipleLightsInSelection())
        {
            ExtraLightDetailBox->SetContent(GroupControls());
        }
        else
            ExtraLightDetailBox->SetContent(DMXChannelProperties());
    }
    else
        ExtraLightDetailBox->SetContent(SNew(SBox));
}

void SDMXControlTool::ClearSelection()
{
    if (TreeWidget)
    {
        ToolData->SelectedItems.Empty();
        TreeWidget->Tree->ClearSelection();
        ToolData->SelectionMasterLight = nullptr;
        ToolData->LightsUnderSelection.Empty();
    }
    //UpdateLightHeader();
    UpdateExtraLightDetailBox();
    LightSpecificWidget->UpdateToolState();
}


TSharedRef<SBox> SDMXControlTool::GroupControls()
{
    TSharedPtr<SBox> Box;


    SVerticalBox::FSlot* MasterLightSlot;
    SAssignNew(Box, SBox)
        [
            SNew(SBorder)
            .Padding(FMargin(5.0f, 5.0f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .Expose(MasterLightSlot)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                    SNew(STextBlock)
                    .Text(FText::FromString("Master Light"))
                    ]
                    + SHorizontalBox::Slot()
                    [
                       SNew(SComboBox<UItemHandle*>)
                       .OptionsSource(&ToolData->LightsUnderSelection)
                       .OnGenerateWidget(this, &SDMXControlTool::GroupControlDropDownLabel)
                       .OnSelectionChanged(this, &SDMXControlTool::GroupControlDropDownSelection)
                       .InitiallySelectedItem(ToolData->SelectionMasterLight)
                       [
                           SNew(STextBlock).Text(this, &SDMXControlTool::GroupControlDropDownDefaultLabel)
                       ]
                   ]
                ]
                + SVerticalBox::Slot()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Affected lights"))
                    ]
                    + SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Text(this, &SDMXControlTool::GroupControlLightList)
                        .AutoWrapText(true)
                    ]
                ]
            ]
        ];

    MasterLightSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


    return Box.ToSharedRef();
}

TSharedRef<SBox> SDMXControlTool::DMXChannelProperties()
{
    TSharedPtr<SBox> Box;


    auto MasterLightHandle = ToolData->GetMasterLight();
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

    auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);
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
    auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);
    return MasterDMX->bDMXEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SDMXControlTool::UseDMXCheckboxStateChanged(ECheckBoxState NewState)
{
    auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);

    GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX usage changed"));
    MasterDMX->BeginTransaction();
    MasterDMX->bDMXEnabled = NewState == ECheckBoxState::Checked;
    MasterDMX->UpdateDMXChannels();
    GEditor->EndTransaction();
}

FString SDMXControlTool::DMXConfigObjectPath() const
{
    auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);

    FString ObjectPath = "";
    if (MasterDMX->Config)
    {
        ObjectPath = MasterDMX->Config->GetAssetPath();
    }
    return ObjectPath;
}

void SDMXControlTool::OnSetDMXConfigAsset(const FAssetData& AssetData)
{
    auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);
    GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX config changed"));
    MasterDMX->BeginTransaction();
    MasterDMX->Config = DuplicateObject(Cast<UDMXConfigAsset>(AssetData.GetAsset()), MasterDMX);
    MasterDMX->Config->AssetName = AssetData.AssetName;
    //MasterDMX->Config
}

TOptional<int> SDMXControlTool::StartingChannelBoxGetValue() const
{
    auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);
    return MasterDMX->StartingChannel;
}

void SDMXControlTool::StartingChannelBoxValueCommitted(int Value, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::Default)
    {
        auto MasterDMX = Cast<UDMXLight>(ToolData->GetMasterLight()->Item);
        GEditor->BeginTransaction(FText::FromString(MasterDMX->Handle->Name + "DMX starting channel changed"));
        MasterDMX->BeginTransaction();
        MasterDMX->StartingChannel = Value;
        MasterDMX->UpdateDMXChannels();
        GEditor->EndTransaction();
    }
}

TSharedRef<SWidget> SDMXControlTool::GroupControlDropDownLabel(UItemHandle* Item)
{
    if (Item->Type == ETreeItemType::Folder)
    {
        return SNew(SBox);
    }
    return SNew(STextBlock).Text(FText::FromString(Item->Name));
}

void SDMXControlTool::GroupControlDropDownSelection(UItemHandle* Item, ESelectInfo::Type SelectInfoType)
{
    ToolData->SelectionMasterLight = Item;
    LightSpecificWidget->UpdateToolState();
}

FText SDMXControlTool::GroupControlDropDownDefaultLabel() const
{
    if (ToolData->SelectionMasterLight)
    {
        return FText::FromString(ToolData->SelectionMasterLight->Name);
    }
    return FText::FromString("");
}

FText SDMXControlTool::GroupControlLightList() const
{
    FString LightList = ToolData->LightsUnderSelection[0]->Name;

    for (size_t i = 1; i < ToolData->LightsUnderSelection.Num(); i++)
    {
        LightList += ", ";
        LightList += ToolData->LightsUnderSelection[i]->Name;
    }

    return FText::FromString(LightList);
}

SHorizontalBox::FSlot& SDMXControlTool::LightSpecificPropertyEditor()
{
    auto& Slot = SHorizontalBox::Slot();
    Slot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


    Slot
        .Padding(5.0f, 0.0f, 0.0f, 0.0f)
        [
            SAssignNew(LightSpecificWidget, SLightSpecificProperties)
            .ToolData(ToolData)
        ];

    return Slot;
}

#undef LOCTEXT_NAMESPACE