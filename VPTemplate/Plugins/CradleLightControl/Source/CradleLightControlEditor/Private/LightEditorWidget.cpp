#include "LightEditorWidget.h"

#include "DetailWidgetRow.h"
#include "Kismet/GameplayStatics.h"

#include "ToolData.h"
#include "ItemHandle.h"
#include "BaseLight.h"

#include "CradleLightControlEditor.h"
#include "EditorData.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"

void SLightEditorWidget::Construct(const FArguments& Args, UToolData* InToolData, EDataSet InDataSet)
{
    ToolTab = Args._ToolTab;

    DataSet = InDataSet;

    if (!EditorData)
    {
		EditorData = NewObject<UEditorData>();	    
    }
    EditorData->SetToolData(InToolData);
    EditorData->SetWidgetRef(*this);
    EditorData->OpenFileDialog = FLightJsonFileDialogDelegate::CreateRaw(this, &SLightEditorWidget::OpenFileDialog);
    EditorData->SaveFileDialog = FLightJsonFileDialogDelegate::CreateRaw(this, &SLightEditorWidget::SaveFileDialog);

    EditorData->GetToolData()->PostLightTransacted.BindUObject(EditorData, &UEditorData::PostLightTransacted);
    EditorData->GetToolData()->TreeStructureChangedDelegate.BindLambda([this]()
        {
	        if (LightHierarchyWidget && LightHierarchyWidget->Tree)
				LightHierarchyWidget->Tree->RequestTreeRefresh();
        });
    
    EditorData->LoadMetaData();      

    DataAutoSaveTimer = RegisterActiveTimer(300.0f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
        {
            EditorData->AutoSave();

            return EActiveTimerReturnType::Continue;
        }));

    EditorData->AddToRoot();

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
                    SAssignNew(HierarchyVerticalBox, SVerticalBox)
                    +SVerticalBox::Slot()
                    [
                        SAssignNew(LightHierarchyWidget, SLightHierarchyWidget)
                        .EditorData(EditorData)
                        .Name(InToolData->DataName)
                        .SelectionChangedDelegate(FTreeSelectionChangedDelegate::CreateRaw(this, &SLightEditorWidget::OnTreeSelectionChanged))
						.DataVerificationInterval(2.0f)
                    ]
                ]
				+ SSplitter::Slot()
                [                   
                    SAssignNew(PropertiesVerticalBox, SVerticalBox)
                    + LightHeader()
				    + LightPropertyEditor()                   
                ]
            ]
        ];

    LightHierarchyWidget->Tree->RequestTreeRefresh();
    LightHeaderWidget->TreeHierarchyWidget = LightHierarchyWidget;
}

SLightEditorWidget::~SLightEditorWidget()
{
    //PreDestroy();
}

void SLightEditorWidget::PreDestroy()
{
    EditorData->AutoSave();
    if (LightHierarchyWidget)
        LightHierarchyWidget->PreDestroy();
    if (LightPropertyWidget)
        LightPropertyWidget->PreDestroy();
    
    UnRegisterActiveTimer(DataAutoSaveTimer.ToSharedRef());
}

void SLightEditorWidget::OnTreeSelectionChanged()
{
    if (EditorData->IsAMasterLightSelected())
    {
        LightPropertyWidget->UpdateSaturationGradient(EditorData->SelectionMasterLight->Item->Hue);
        UpdateExtraLightDetailBox();
        LightHeaderWidget->Update();
        LightSpecificPropertiesWidget->UpdateToolState();
    }

}

TWeakPtr<SLightHierarchyWidget> SLightEditorWidget::GetTreeWidget()
{
    return LightHierarchyWidget;
}

TWeakPtr<SLightPropertyEditor> SLightEditorWidget::GetLightPropertyEditor()
{
    return LightPropertyWidget;
}

FString SLightEditorWidget::OpenFileDialog(FString Title, FString StartingPath)
{

    TArray<FString> Res;
    if (FCradleLightControlEditorModule::OpenFileDialog(Title, ToolTab->GetParentWindow()->GetNativeWindow()->GetOSWindowHandle(),
        StartingPath, EFileDialogFlags::None, "JSON Data Table|*.json", Res))
    {
        return Res[0];
    }
    return "";
}

FString SLightEditorWidget::SaveFileDialog(FString Title, FString StartingPath)
{
    TArray<FString> Res;
    if (FCradleLightControlEditorModule::SaveFileDialog(Title, ToolTab->GetParentWindow()->GetNativeWindow()->GetOSWindowHandle(),
        StartingPath, EFileDialogFlags::None, "JSON Data Table|*.json", Res))
    {
        return Res[0];
    }
    return "";
}

UEditorData* SLightEditorWidget::GetEditorData() const
{
    return EditorData;
}

TSharedRef<SDockTab> SLightEditorWidget::Show()
{
    if (!ToolTab)
    {
        ToolTab = SNew(SDockTab)
            .Label(FText::FromString(EditorData->GetToolData()->DataName))
            .TabRole(ETabRole::NomadTab)
            .OnTabClosed_Lambda([this](TSharedRef<SDockTab>)
                {
                    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("LightControl");
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

void SLightEditorWidget::Hide()
{
}

void SLightEditorWidget::UpdateSaturationGradient(float NewHueValue)
{
    LightPropertyWidget->UpdateSaturationGradient(NewHueValue);
}

SVerticalBox::FSlot& SLightEditorWidget::LightHeader()
{
    auto& Slot = SVerticalBox::Slot();

    Slot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    Slot
        .HAlign(HAlign_Fill)
        [
            SAssignNew(LightHeaderWidget, SLightItemHeader)
            .EditorData(EditorData)
			.TreeHierarchyWidget(LightHierarchyWidget)
        ];

    //UpdateLightHeader();

    return Slot;

}

SVerticalBox::FSlot& SLightEditorWidget::LightPropertyEditor()
{
    auto& Slot = SVerticalBox::Slot();

    TSharedPtr<SVerticalBox> Box;

    SVerticalBox::FSlot* ExtraLightBoxSlot;

    Slot
    .Padding(20.0f, 30.0f, 20.0f, 0.0f)
    .VAlign(VAlign_Fill)
    .HAlign(HAlign_Fill)
    [
        SAssignNew(PropertiesHorizontalBox, SHorizontalBox)
        + SHorizontalBox::Slot() // General light properties + extra light properties or group controls
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            [
                SAssignNew(LightPropertyWidget, SLightPropertyEditor, EditorData, DataSet)
                .EditorData(EditorData)
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

void SLightEditorWidget::UpdateExtraLightDetailBox()
{
}

void SLightEditorWidget::ClearSelection()
{
    if (LightHierarchyWidget)
    {
        EditorData->SelectedItems.Empty();
        LightHierarchyWidget->Tree->ClearSelection();
        EditorData->SelectionMasterLight = nullptr;
        EditorData->LightsUnderSelection.Empty();
    }
    LightHeaderWidget->Update();
    //UpdateLightHeader();
    UpdateExtraLightDetailBox();
    LightSpecificPropertiesWidget->UpdateToolState();
}

SHorizontalBox::FSlot& SLightEditorWidget::LightSpecificPropertyEditor()
{
    auto& Slot = SHorizontalBox::Slot();
    Slot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


    Slot
        .Padding(5.0f, 0.0f, 0.0f, 0.0f)
        [
            SAssignNew(LightSpecificPropertiesWidget, SLightSpecificProperties, EditorData, DataSet)
            .EditorData(EditorData)
        ];

    return Slot;
}

