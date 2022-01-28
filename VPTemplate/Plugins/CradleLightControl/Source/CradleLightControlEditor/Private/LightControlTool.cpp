#include "LightControlTool.h"

#include "Slate.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Chaos/AABB.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"

#include "ClassIconFinder.h"

#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"

#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"

#include "ToolData.h"
#include "ItemHandle.h"
#include "BaseLight.h"

#include "CradleLightControlEditor.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"

#include "GelPaletteWidget.h"

#include "VirtualLight.h"



void SLightControlTool::Construct(const FArguments& Args, UToolData* InToolData)
{
    

    LoadResources();

    ToolTab = Args._ToolTab;
    ToolData = InToolData;
    ToolData->DataName = "VirtualLight";
    ToolData->OpenFileDialog = FLightJsonFileDialogDelegate::CreateRaw(this, &SLightControlTool::OpenFileDialog);
    ToolData->SaveFileDialog = FLightJsonFileDialogDelegate::CreateRaw(this, &SLightControlTool::SaveFileDialog);
    ToolData->MasterLightTransactedDelegate = FOnMasterLightTransactedDelegate::CreateLambda([this](UItemHandle* ItemHandle)
        {
            LightPropertyWidget->UpdateSaturationGradient(ItemHandle->Item->GetHue());
        });

    ToolData->MetaDataSaveExtension = FMetaDataExtension::CreateRaw(this, &SLightControlTool::MetaDataSaveExtension);
    ToolData->MetaDataLoadExtension = FMetaDataExtension::CreateRaw(this, &SLightControlTool::MetaDataLoadExtension);
    
    ToolData->LoadMetaData();


    // Build a database from what is in the level if there wasn't a file to recover from
    if (ToolData->RootItems.Num() == 0)
    {
        UpdateLightList();
    }

    DataAutoSaveTimer = RegisterActiveTimer(300.0f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
        {
            ToolData->AutoSave();

            return EActiveTimerReturnType::Continue;
        }));

    ToolData->AddToRoot();

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
        //.SizeRule(SSplitter::ESizeRule::SizeToContent)
        [
            SAssignNew(TreeWidget, SLightTreeHierarchy)
            .ToolData(ToolData)
            .Name("Virtual Lights")
            .DataVerificationDelegate(FItemDataVerificationDelegate::CreateRaw(this, &SLightControlTool::VerifyTreeData))
            .DataVerificationInterval(2.0f)
            .DataUpdateDelegate(FUpdateItemDataDelegate::CreateStatic(&SLightControlTool::UpdateItemData))
            .SelectionChangedDelegate(FTreeSelectionChangedDelegate::CreateRaw(this, &SLightControlTool::OnTreeSelectionChanged))
            ]
            + SSplitter::Slot()
            [
                SNew(SHorizontalBox)
                /*+ SHorizontalBox::Slot()
                .Expose(SeparatorSlot)
                .Padding(0.0f, 0.0f, 30.0f, 0.0f)
                [
                    SNew(SSeparator)
                    .Orientation(EOrientation::Orient_Vertical)
                ]         */
                + SHorizontalBox::Slot()
                [
                    SNew(SVerticalBox)                
                    + LightHeader()
                    + LightPropertyEditor()
					
                ]
            ]
        ]
    ];


    

    FWorldDelegates::OnWorldCleanup.AddLambda([this](UWorld*, bool, bool)
        {
            ToolData->SaveMetaData();
            if (ActorSpawnedListenerHandle.IsValid())
            {
                GWorld->RemoveOnActorSpawnedHandler(ActorSpawnedListenerHandle);
                ActorSpawnedListenerHandle.Reset();
            }
        });

    FEditorDelegates::OnMapOpened.AddLambda([this](const FString&, bool)
    {
    /*OnWorldChangedDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddLambda([this](UWorld*, const UWorld::InitializationValues&)
        {*/
            ClearSelection();
			// Load the metadata for the current world
            ToolData->LoadMetaData();
            
			// If there is no concrete file associated with the current world, generate the data from scratch.
			// This way we avoid the tool using invalid data 
            if (ToolData->ToolPresetPath.IsEmpty())
            {
                UpdateLightList();
            }

            // TODO: This must be done even without the tool tab being spawned, so ideally on an event that signals the end of the initialization of the level
            if (!ActorSpawnedListenerHandle.IsValid() && GWorld)
            {
                auto ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateRaw(this, &SLightControlTool::ActorSpawnedCallback);
                ActorSpawnedListenerHandle = GWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
            }
            else
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Could not set actor spawned callback");

            ToolData->bCurrentlyLoading = false;
        });
}

SLightControlTool::~SLightControlTool()
{
    //PreDestroy();
}

void SLightControlTool::PreDestroy()
{
    ToolData->AutoSave();
    if (TreeWidget)
        TreeWidget->PreDestroy();
    if (LightPropertyWidget)
        LightPropertyWidget->PreDestroy();

    FWorldDelegates::OnPostWorldInitialization.Remove(OnWorldChangedDelegateHandle);
    FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanupStartedDelegate);
    GWorld->RemoveOnActorSpawnedHandler(ActorSpawnedListenerHandle);
    UnRegisterActiveTimer(DataAutoSaveTimer.ToSharedRef());
}

void SLightControlTool::ActorSpawnedCallback(AActor* Actor)
{
    TreeWidget->OnActorSpawned(Actor);
}

void SLightControlTool::OnTreeSelectionChanged()
{
    if (ToolData->IsAMasterLightSelected())
    {
        LightPropertyWidget->UpdateSaturationGradient(ToolData->SelectionMasterLight->Item->Hue);
        UpdateExtraLightDetailBox();
        ItemHeader->Update();
        LightSpecificWidget->UpdateToolState();
    }

}

TWeakPtr<SLightTreeHierarchy> SLightControlTool::GetTreeWidget()
{
    return TreeWidget;
}

TWeakPtr<SLightPropertyEditor> SLightControlTool::GetLightPropertyEditor()
{
    return LightPropertyWidget;
}

FString SLightControlTool::OpenFileDialog(FString Title, FString StartingPath)
{

    TArray<FString> Res;
    if (FCradleLightControlEditorModule::OpenFileDialog(Title, ToolTab->GetParentWindow()->GetNativeWindow()->GetOSWindowHandle(),
        StartingPath, EFileDialogFlags::None, "JSON Data Table|*.json", Res))
    {
        return Res[0];
    }
    return "";
}

FString SLightControlTool::SaveFileDialog(FString Title, FString StartingPath)
{
    TArray<FString> Res;
    if (FCradleLightControlEditorModule::SaveFileDialog(Title, ToolTab->GetParentWindow()->GetNativeWindow()->GetOSWindowHandle(),
        StartingPath, EFileDialogFlags::None, "JSON Data Table|*.json", Res))
    {
        return Res[0];
    }
    return "";
}

void SLightControlTool::UpdateLightList()
{
    ToolData->ClearAllData();
    TArray<AActor*> Actors;
    // Fetch Point Lights
    UGameplayStatics::GetAllActorsOfClass(GWorld, APointLight::StaticClass(), Actors);
    for (auto Light : Actors)
    {
        auto* NewItem = Cast<UVirtualLight>(ToolData->AddItem()->Item);
        NewItem->Handle->Type = ETreeItemType::PointLight;
        NewItem->Handle->Name = Light->GetName();
        NewItem->PointLight = Cast<APointLight>(Light);
        UpdateItemData(NewItem->Handle);

        ToolData->RootItems.Add(NewItem->Handle);
        TreeWidget->GenerateWidgetForItem(NewItem->Handle);
    }

    // Fetch Sky Lights
    UGameplayStatics::GetAllActorsOfClass(GWorld, ASkyLight::StaticClass(), Actors);
    for (auto Light : Actors)
    {
        auto* NewItem = Cast<UVirtualLight>(ToolData->AddItem()->Item);
        NewItem->Handle->Type = ETreeItemType::SkyLight;
        NewItem->Handle->Name = Light->GetName();
        NewItem->SkyLight = Cast<ASkyLight>(Light);
        UpdateItemData(NewItem->Handle);

        ToolData->RootItems.Add(NewItem->Handle);
        TreeWidget->GenerateWidgetForItem(NewItem->Handle);
    }

    // Fetch Directional Lights
    UGameplayStatics::GetAllActorsOfClass(GWorld, ADirectionalLight::StaticClass(), Actors);
    for (auto Light : Actors)
    {
        auto* NewItem = Cast<UVirtualLight>(ToolData->AddItem()->Item);

        NewItem->Handle->Type = ETreeItemType::DirectionalLight;
        NewItem->Handle->Name = Light->GetName();
        NewItem->DirectionalLight = Cast<ADirectionalLight>(Light);
        UpdateItemData(NewItem->Handle);

        ToolData->RootItems.Add(NewItem->Handle);
        TreeWidget->GenerateWidgetForItem(NewItem->Handle);
    }

    // Fetch Spot Lights
    UGameplayStatics::GetAllActorsOfClass(GWorld, ASpotLight::StaticClass(), Actors);
    for (auto Light : Actors)
    {
        auto* NewItem = Cast<UVirtualLight>(ToolData->AddItem()->Item);

        NewItem->Handle->Type = ETreeItemType::SpotLight;
        NewItem->Handle->Name = Light->GetName();
        NewItem->SpotLight = Cast<ASpotLight>(Light);
        UpdateItemData(NewItem->Handle);

        ToolData->RootItems.Add(NewItem->Handle);
        TreeWidget->GenerateWidgetForItem(NewItem->Handle);
    }

    TreeWidget->Tree->RequestTreeRefresh();
}

void SLightControlTool::UpdateItemData(UItemHandle* ItemHandle)
{
    check(ItemHandle->Type != Folder);
    auto Item = Cast<UVirtualLight>(ItemHandle->Item);
    FLinearColor RGB;

    Item->Intensity = 0.0f;
    Item->Saturation = 0.0f;
    Item->Temperature = 0.0f;

    if (ItemHandle->Type == ETreeItemType::SkyLight)
    {
        RGB = Item->SkyLight->GetLightComponent()->GetLightColor();
        Item->bIsEnabled = Item->SkyLight->GetLightComponent()->IsVisible();
    }
    else
    {
        ALight* LightPtr = Cast<ALight>(Item->ActorPtr);
        RGB = LightPtr->GetLightColor();
        Item->bIsEnabled = LightPtr->GetLightComponent()->IsVisible();
    }
    auto HSV = RGB.LinearRGBToHSV();
    Item->Saturation = HSV.G;

    // If Saturation is 0, the color is white. The RGB => HSV conversion calculates the Hue to be 0 in that case, even if it's not supposed to be.
    // Do this to preserve the Hue previously used rather than it getting reset to 0.
    if (Item->Saturation != 0.0f)
        Item->Hue = HSV.R;

    if (ItemHandle->Type == ETreeItemType::PointLight)
    {
        auto Comp = Item->PointLight->PointLightComponent;
        Item->Intensity = Comp->Intensity;
    }
    else if (ItemHandle->Type == ETreeItemType::SpotLight)
    {
        auto Comp = Item->SpotLight->SpotLightComponent;
        Item->Intensity = Comp->Intensity;
    }

    if (ItemHandle->Type != ETreeItemType::SkyLight)
    {
        auto LightPtr = Cast<ALight>(Item->ActorPtr);
        auto LightComp = LightPtr->GetLightComponent();
        Item->bUseTemperature = LightComp->bUseTemperature;
        Item->Temperature = LightComp->Temperature;

        Item->bCastShadows = LightComp->CastShadows;
    }
    else
    {
        Item->bCastShadows = Item->SkyLight->GetLightComponent()->CastShadows;
    }

    auto CurrentFwd = FQuat::MakeFromEuler(FVector(0.0f, Item->Vertical, Item->Horizontal)).GetForwardVector();
    auto ActorQuat = Item->ActorPtr->GetTransform().GetRotation().GetNormalized();
    auto ActorFwd = ActorQuat.GetForwardVector();

    if (CurrentFwd.Equals(ActorFwd))
    {
        auto Euler = ActorQuat.Euler();
        Item->Horizontal = Euler.Z;
        Item->Vertical = Euler.Y;
    }


    if (ItemHandle->Type == ETreeItemType::SpotLight)
    {
        Item->InnerAngle = Item->SpotLight->SpotLightComponent->InnerConeAngle;
        Item->OuterAngle = Item->SpotLight->SpotLightComponent->OuterConeAngle;
    }

}

void SLightControlTool::VerifyTreeData()
{
    if (ToolData->bCurrentlyLoading)
        return;
    
    GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, "Cleaning invalid lights");
    TArray<UItemHandle*> ToRemove;
    for (auto ItemHandle : ToolData->ListOfLightItems)
    {
        auto Item = Cast<UVirtualLight>(ItemHandle->Item);
        if (!Item->ActorPtr || !IsValid(Item->SkyLight))
        {
            if (ItemHandle->Parent)
                ItemHandle->Parent->Children.Remove(ItemHandle);
            else
                ToolData->RootItems.Remove(ItemHandle);


            ToRemove.Add(ItemHandle);
        }
        else
        {
            UpdateItemData(ItemHandle);
        }
    }

    for (auto Item : ToRemove)
    {
        ToolData->ListOfTreeItems.Remove(Item);
        ToolData->ListOfLightItems.Remove(Item);
    }

    if (ToRemove.Num())
    {
        TreeWidget->Tree->RequestTreeRefresh();
    }
}

UToolData* SLightControlTool::GetToolData() const
{
	return ToolData;
}

TSharedRef<SDockTab> SLightControlTool::Show()
{
    if (!ToolTab)
    {
        ToolTab = SNew(SDockTab)
            .Label(FText::FromString("Virtual Light Control"))
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

void SLightControlTool::Hide()
{
}


void SLightControlTool::LoadResources()
{
    
}

SVerticalBox::FSlot& SLightControlTool::LightHeader()
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

void SLightControlTool::MetaDataSaveExtension(TSharedPtr<FJsonObject> RootJson)
{
	if (GWorld)
	{
        auto OpenedJson = ToolData->OpenMetaDataJson();
        if (OpenedJson)
            *RootJson = *OpenedJson; // Replace whatever was done by the default saving with the current file state
        else
            *RootJson = FJsonObject();

        auto MapName = GWorld->GetMapName();

        RootJson->SetStringField(MapName, ToolData->ToolPresetPath);
		
	}


}

void SLightControlTool::MetaDataLoadExtension(TSharedPtr<FJsonObject> RootJson)
{
	if (GWorld)
	{
        auto MapName = GWorld->GetMapName();

        if (RootJson->HasField(MapName))
        {
            ToolData->ToolPresetPath = RootJson->GetStringField(MapName);	        
        }
	}
}


SVerticalBox::FSlot& SLightControlTool::LightPropertyEditor()
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

void SLightControlTool::UpdateExtraLightDetailBox()
{
    if (ToolData->IsAMasterLightSelected())
    {
        if (ToolData->MultipleLightsInSelection())
        {
            ExtraLightDetailBox->SetContent(GroupControls());
        }
        else
        {
            ExtraLightDetailBox->SetContent(LightTransformViewer());
        }
    }
    else
        ExtraLightDetailBox->SetContent(SNew(SBox));
}

void SLightControlTool::ClearSelection()
{
    if (TreeWidget)
    {
        ToolData->SelectedItems.Empty();
        TreeWidget->Tree->ClearSelection();
        ToolData->SelectionMasterLight = nullptr;
        ToolData->LightsUnderSelection.Empty();
    }
    ItemHeader->Update();
    //UpdateLightHeader();
    UpdateExtraLightDetailBox();
    LightSpecificWidget->UpdateToolState();
}


TSharedRef<SBox> SLightControlTool::LightTransformViewer()
{
    SHorizontalBox::FSlot* ButtonsSlot;

    TSharedPtr<SBox> Box;

    SAssignNew(Box, SBox)
    [
        SNew(SBorder)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .HAlign(HAlign_Fill)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Parent Object"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemParentName)
                    ]
                ]
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Position"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemPosition)
                    ]
                ]
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Rotation"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemRotation)
                    ]
                ]
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Scale"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemScale)
                    ]
                ]
            ]
            +SHorizontalBox::Slot()
            .Expose(ButtonsSlot)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Select Scene Object"))
                    .OnClicked(this, &SLightControlTool::SelectItemInScene)
                ]
                + SVerticalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Select Parent Object"))
                    .IsEnabled(this, &SLightControlTool::SelectItemParentButtonEnable)
                    .OnClicked(this, &SLightControlTool::SelectItemParent)
                ]
            ]
        ]
    ];

    ButtonsSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    return Box.ToSharedRef();
}

FReply SLightControlTool::SelectItemInScene()
{
    if (ToolData->IsAMasterLightSelected())
    {
        GEditor->SelectNone(true, true);
        GEditor->SelectActor(Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr, true, true, false, true);
    }

    return FReply::Handled();
}

FReply SLightControlTool::SelectItemParent()
{
    GEditor->SelectNone(true, true);
    GEditor->SelectActor(Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor(), true, true, false, true);

    return FReply::Handled();
}

bool SLightControlTool::SelectItemParentButtonEnable() const
{
    return ToolData->IsAMasterLightSelected() && Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor();
}

FText SLightControlTool::GetItemParentName() const
{
    if (ToolData->IsAMasterLightSelected() && Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor())
    {
        return FText::FromString(Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor()->GetName());
    }
    return FText::FromString("None");
}

FText SLightControlTool::GetItemPosition() const
{
    if (ToolData->IsAMasterLightSelected())
    {
        return FText::FromString(Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetActorLocation().ToString());
    }
    return FText::FromString("");
}

FText SLightControlTool::GetItemRotation() const
{
    if (ToolData->IsAMasterLightSelected())
    {
        return FText::FromString(Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetActorRotation().ToString());
    }
    return FText::FromString("");
}

FText SLightControlTool::GetItemScale() const
{
    if (ToolData->IsAMasterLightSelected())
    {
        return FText::FromString(Cast<UVirtualLight>(ToolData->SelectionMasterLight->Item)->ActorPtr->GetActorScale().ToString());
    }
    return FText::FromString("");
}

TSharedRef<SBox> SLightControlTool::GroupControls()
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
                    .OnGenerateWidget(this, &SLightControlTool::GroupControlDropDownLabel)
                    .OnSelectionChanged(this, &SLightControlTool::GroupControlDropDownSelection)
                    .InitiallySelectedItem(ToolData->SelectionMasterLight)[
                        SNew(STextBlock).Text(this, &SLightControlTool::GroupControlDropDownDefaultLabel)
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
                    .Text(this, &SLightControlTool::GroupControlLightList)
                    .AutoWrapText(true)
                ]
            ]
        ]
    ];

    MasterLightSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


    return Box.ToSharedRef();
}

TSharedRef<SWidget> SLightControlTool::GroupControlDropDownLabel(UItemHandle* Item)
{
    if (Item->Type == ETreeItemType::Folder)
    {
        return SNew(SBox);
    }
    return SNew(STextBlock).Text(FText::FromString(Item->Name));
}

void SLightControlTool::GroupControlDropDownSelection(UItemHandle* Item, ESelectInfo::Type SelectInfoType)
{
    ToolData->SelectionMasterLight = Item;
    LightSpecificWidget->UpdateToolState();
}

FText SLightControlTool::GroupControlDropDownDefaultLabel() const
{
    if (ToolData->SelectionMasterLight)
    {
        return FText::FromString(ToolData->SelectionMasterLight->Name);
    }
    return FText::FromString("");
}

FText SLightControlTool::GroupControlLightList() const
{
    FString LightList = ToolData->LightsUnderSelection[0]->Name;

    for (size_t i = 1; i < ToolData->LightsUnderSelection.Num(); i++)
    {
        LightList += ", ";
        LightList += ToolData->LightsUnderSelection[i]->Name;
    }

    return FText::FromString(LightList);
}

SHorizontalBox::FSlot& SLightControlTool::LightSpecificPropertyEditor()
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

