#include "LightTreeHierarchy.h"

#include "CradleLightControlEditor.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"

#include "Widgets/Layout/SScaleBox.h"

#include "Styling/SlateIconFinder.h"
#include "ItemHandle.h"
#include "EditorData.h"
#include "LightControlLoadingResult.h"
#include "ToolData.h"
#include "TreeItemWidget.h"

#include "LightControlTool.h"

#include "VirtualLight.h"

void SLightHierarchyWidget::Construct(const FArguments& Args)
{
    



    SaveIcon = FSlateIconFinder::FindIcon("AssetEditor.SaveAsset");
    SaveAsIcon = FSlateIconFinder::FindIcon("AssetEditor.SaveAssetAs");
    LoadIcon = FSlateIconFinder::FindIcon("EnvQueryEditor.Profiler.LoadStats");

    FSlateFontInfo Font24(FCoreStyle::GetDefaultFont(), 20);
    _ASSERT(Args._EditorData);
    EditorData = Args._EditorData;
    DataUpdateDelegate = Args._DataUpdateDelegate;
    DataVerificationDelegate = Args._DataVerificationDelegate;
    SelectionChangedDelegate = Args._SelectionChangedDelegate;
    HeaderText = FText::FromString(Args._Name);

    EditorData->OnToolDataLoaded = FOnToolDataLoadedDelegate::CreateRaw(this, &SLightHierarchyWidget::OnToolDataLoadedCallback);
    EditorData->LoadMetaData();
    EditorData->ItemExpansionChangedDelegate = FItemExpansionChangedDelegate::CreateLambda([this](UItemHandle* Item, bool bState)
        {
            GetWidgetForItem(Item)->bExpanded = bState;
            UpdateExpansionForItem(Item, false);
        });

    EditorData->TreeStructureChangedDelegate = FOnTreeStructureChangedDelegate::CreateLambda([this]()
        {
            ItemWidgets.Empty();
			for (auto& Item : EditorData->ListOfTreeItems)
			{
                ItemWidgets.FindOrAdd(Item) = SNew(STreeItemWidget, Item, EditorData);
			}
    		Tree->RequestTreeRefresh();
        });

	LightVerificationTimer = RegisterActiveTimer(Args._DataVerificationInterval, FWidgetActiveTimerDelegate::CreateRaw(this, &SLightHierarchyWidget::VerifyLights));

    // SVerticalBox slots are by default dividing the space equally between each other
    // Because of this we need to expose the slot with the search bar in order to disable that for it

    SHorizontalBox::FSlot* SaveButtonSlot;
    SHorizontalBox::FSlot* SaveAsButtonSlot;
    SHorizontalBox::FSlot* LoadButtonSlot;

    SVerticalBox::FSlot* LightSearchBarSlot;
    SVerticalBox::FSlot* NewFolderButtonSlot;

    ChildSlot[
        SNew(SVerticalBox) // Light selection menu thingy
        +SVerticalBox::Slot()
        .Expose(LightSearchBarSlot)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .VAlign(VAlign_Top)
            [
                SNew(SSearchBox)
                //.OnSearch(this, &SLightHierarchyWidget::SearchBarSearch)
                .OnTextChanged(this, &SLightHierarchyWidget::SearchBarOnChanged)
                // Search bar for light
            ]
            +SVerticalBox::Slot()
            .VAlign(VAlign_Top)
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(HeaderText)
                .Font(Font24)
            ]
            +SVerticalBox::Slot()
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightHierarchyWidget::GetPresetFilename)
                        .Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 14))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Right)
                    .VAlign(VAlign_Center)
                    .Expose(SaveButtonSlot)
                    [
                        SNew(SButton)
                        .ButtonColorAndOpacity(FSlateColor(FColor::Transparent))
                        .OnClicked_UObject(EditorData, &UEditorData::SaveCallBack)
                        .RenderTransform(FSlateRenderTransform(0.9f))
                        .ToolTipText(FText::FromString("Save"))
                        [
                            SNew(SOverlay)
                            +SOverlay::Slot()
                            [
                                SNew(SImage)
                                .RenderOpacity(1.0f)
                                .Image(SaveIcon.GetIcon())
                            ]
                        ]
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Right)
                    .VAlign(VAlign_Center)
                    .Expose(SaveAsButtonSlot)
                    [
                        SNew(SButton)
                        .ButtonColorAndOpacity(FSlateColor(FColor::Transparent))
                        .OnClicked_UObject(EditorData, &UEditorData::SaveAsCallback)
                        .RenderTransform(FSlateRenderTransform(0.9f))
                        .ToolTipText(FText::FromString("Save As"))
                        [
                            SNew(SImage)
                            .Image(SaveAsIcon.GetIcon())
                        ]
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Right)
                    .VAlign(VAlign_Center)
                    .Expose(LoadButtonSlot)
                    [
                        SNew(SButton)
                        .ButtonColorAndOpacity(FSlateColor(FColor::Transparent))
                        .OnClicked_UObject(EditorData, &UEditorData::LoadCallBack)
                        .RenderTransform(FSlateRenderTransform(0.9f))
                        .ToolTipText(FText::FromString("Load"))
                        [
                            SNew(SImage)
                            .Image(LoadIcon.GetIcon())
                        ]
                    ]
                ]
        ]
        +SVerticalBox::Slot()
        .Padding(0.0f, 0.0f, 8.0f, 0.0f)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        [
            SNew(SBox)
            .VAlign(VAlign_Top)
            .HAlign(HAlign_Fill)
            [
                SAssignNew(Tree, STreeView<UItemHandle*>)
                .ItemHeight(24.0f)
                .TreeItemsSource(&EditorData->RootItems)
                .OnSelectionChanged(this, &SLightHierarchyWidget::SelectionCallback)
                .OnGenerateRow(this, &SLightHierarchyWidget::AddToTree)
                .OnGetChildren(this, &SLightHierarchyWidget::GetTreeItemChildren)
                .OnExpansionChanged(this, &SLightHierarchyWidget::ExpansionChangedCallback)
                
            ]
        ]
        +SVerticalBox::Slot()
        .VAlign(VAlign_Bottom)
        .Padding(0.0f, 10.0f, 0.0f, 0.0f)
        .Expose(NewFolderButtonSlot)
        [
            SNew(SButton)
            .Text(FText::FromString("Add New Group"))
            .OnClicked_Raw(this, &SLightHierarchyWidget::AddFolderToTree)
        ]
    ];

    SaveButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    SaveAsButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    LoadButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    LightSearchBarSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    NewFolderButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

}

void SLightHierarchyWidget::PreDestroy()
{
    if (LightVerificationTimer)
        UnRegisterActiveTimer(LightVerificationTimer.ToSharedRef());
}

void SLightHierarchyWidget::OnActorSpawned(AActor* Actor)
{
    
}

void SLightHierarchyWidget::BeginTransaction()
{
    EditorData->Modify();
}

void SLightHierarchyWidget::GenerateWidgetForItem(UItemHandle* Item)
{
    ItemWidgets.FindOrAdd(Item) = SNew(STreeItemWidget, Item, EditorData);
}

TSharedRef<ITableRow> SLightHierarchyWidget::AddToTree(UItemHandle* ItemPtr,
                                                     const TSharedRef<STableViewBase>& OwnerTable)
{
    SHorizontalBox::FSlot& CheckBoxSlot = SHorizontalBox::Slot();
    CheckBoxSlot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    auto Widget = GetWidgetForItem(ItemPtr);

    if (!Widget)
    {
	    Widget = SNew(STreeItemWidget, ItemPtr, EditorData);
    }

    ItemWidgets.FindOrAdd(ItemPtr) = Widget;

    auto Row =
        SNew(STableRow<UItemHandle*>, OwnerTable)
        .Padding(2.0f)
        .OnDragDetected_Raw(this, &SLightHierarchyWidget::DragDropBegin)
        .OnDrop_Lambda([ItemPtr, this](const FDragDropEvent& DragDropEvent)
        {
            StaticCastSharedPtr<FItemDragDropOperation>(DragDropEvent.GetOperation())->Destination = ItemPtr;
            return DragDropEnd(DragDropEvent);
        })
        .Visibility_Lambda([Widget]() {return Widget->bMatchesSearchString ? EVisibility::Visible : EVisibility::Collapsed; })
        [
            Widget->AsShared()
        ];

    //ItemPtr->GenerateTableRow();

    return Row;
}

void SLightHierarchyWidget::GetTreeItemChildren(UItemHandle* Item, TArray<UItemHandle*>& Children)
{
    Children.Append(Item->Children);
}

void SLightHierarchyWidget::SelectionCallback(UItemHandle* Item, ESelectInfo::Type SelectType)
{
    // Strangely, this callback is triggered when an redo is done that brings back a deleted group
    // When that happens, the tree widget is considered to have no selected items, which will incorrectly make the masterlight a nullptr
    // In these cases, Item is garbage and invalid, so by verifying the validity we ensure we don't set the masterlight to a nullptr incorrectly. 
    if (IsValid(Item))
    {

        auto Objects = Tree->GetSelectedItems();
        auto& SelectedItems = EditorData->SelectedItems;
        auto& LightsUnderSelection = EditorData->LightsUnderSelection;
        auto& SelectionMasterLight = EditorData->SelectionMasterLight;
        SelectedItems.Empty();

        for (auto Object : Objects)
        {
            SelectedItems.Add(Object);
        }

        if (SelectedItems.Num())
        {
            int Index;
            LightsUnderSelection.Empty();
            for (auto& Selected : SelectedItems)
            {
                Selected->GetLights(LightsUnderSelection);
            }
            if (LightsUnderSelection.Num())
            {
                if (SelectionMasterLight == nullptr || !LightsUnderSelection.Find(SelectionMasterLight, Index))
                {
                    //GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Selection");
                    SelectionMasterLight = LightsUnderSelection[0];
                }
            }
        }
        else
            SelectionMasterLight = nullptr;
        SelectionChangedDelegate.ExecuteIfBound();
    }
}

FReply SLightHierarchyWidget::AddFolderToTree()
{
    UItemHandle* NewFolder = EditorData->AddItem();
    NewFolder->Name = "New Group";
    GenerateWidgetForItem(NewFolder);
    GetWidgetForItem(NewFolder)->CheckNameAgainstSearchString(SearchString);
    EditorData->RootItems.Add(NewFolder);

    Tree->RequestTreeRefresh();


    return FReply::Handled();
}

void SLightHierarchyWidget::OnToolDataLoadedCallback(ELightControlLoadingResult LoadingResult)
{
    if (LoadingResult == ELightControlLoadingResult::Success)
    {
        UE_LOG(LogTemp, Display, TEXT("Light control state loaded successfully"));
    }
    else
    {
        FString ErrorMessage = "";

        switch (LoadingResult)
        {
        case ELightControlLoadingResult::LightNotFound:
            ErrorMessage = "At least one light could not be found. Please ensure all lights exist and haven't been renamed since the w.";
            break;
        case ELightControlLoadingResult::EngineError:
            ErrorMessage = "There was an error with the engine. Please try loading again. If the error persists, restart the engine.";
            break;
        case ELightControlLoadingResult::InvalidType:
            ErrorMessage = "The item type that was tried to be loaded was not valid. Please ensure that the item type in the .json file is between 0 and 4.";
            break;
        case ELightControlLoadingResult::MultipleErrors:
            ErrorMessage = "Multiple errors occurred. See output log for more details.";
            break;
        }

        UE_LOG(LogTemp, Display, TEXT("Light control state could not load with following message: %s"), *ErrorMessage);

        FNotificationInfo NotificationInfo(FText::FromString(FString::Printf(TEXT("Light control tool state could not be loaded. Please check the output log."))));

        NotificationInfo.ExpireDuration = 30.0f;
        NotificationInfo.bUseSuccessFailIcons = false;

        FSlateNotificationManager::Get().AddNotification(NotificationInfo);
    }

    for (auto& Item : EditorData->RootItems)
    {
        RegenerateItemHandleWidgets(Item);
    }
}

void SLightHierarchyWidget::RegenerateItemHandleWidgets(UItemHandle* ItemHandle)
{
    GenerateWidgetForItem(ItemHandle);

    for (auto Child : ItemHandle->Children)
    {
        RegenerateItemHandleWidgets(Child);
    }
}


EActiveTimerReturnType SLightHierarchyWidget::VerifyLights(double, float)
{
	if (DataVerificationDelegate.IsBound())
	{
		DataVerificationDelegate.Execute();		
		return EActiveTimerReturnType::Continue;
	}
    return EActiveTimerReturnType::Stop;
}

FReply SLightHierarchyWidget::DragDropBegin(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
    TSharedRef<FItemDragDropOperation> DragDropOp = MakeShared<FItemDragDropOperation>();
    DragDropOp->DraggedItems = EditorData->GetSelectedItems();
    
    FReply Reply = FReply::Handled();

    Reply.BeginDragDrop(DragDropOp);

    return Reply;
}

FReply SLightHierarchyWidget::DragDropEnd(const FDragDropEvent& DragDropEvent)
{
    auto DragDrop = StaticCastSharedPtr<FItemDragDropOperation>(DragDropEvent.GetOperation());
	UItemHandle* Destination = DragDrop->Destination;
    if (GEditor)
        GEditor->BeginTransaction(FText::FromString("Light control tree drag and drop"));
    for (auto Target : DragDrop->DraggedItems)
    {
        //auto Target = DragDrop->DraggedItem;
        auto Source = Target->Parent;

        if (!UItemHandle::VerifyDragDrop(Target, Destination))
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Drag drop cancelled");
            auto Reply = FReply::Handled();
            Reply.EndDragDrop();

            return FReply::Handled();
        }


        // The source folder and the dragged item will always be affected, so always begin transacting them
        if (Source)
            Source->BeginTransaction(false);
        else
            EditorData->GetToolData()->BeginTransaction();
        Target->BeginTransaction(false);
        Destination->BeginTransaction(false);
        if (!Destination->Item)
        {
            Destination = DragDrop->Destination;

            if (Source)
                Source->Children.Remove(Target);
            else
                EditorData->RootItems.Remove(Target);
            Destination->Children.Add(Target);
            Target->Parent = Destination;
            
            EditorData->ItemExpansionChangedDelegate.ExecuteIfBound(Destination, true);
        }
        else
        {
            auto Parent = Destination->Parent;
            Destination = EditorData->AddItem();
            Destination->Name = DragDrop->Destination->Name + " Group";
            Destination->Parent = Parent;
            Destination->BeginTransaction(false);
            GenerateWidgetForItem(Destination);


            if (Destination->Parent)
                Destination->Parent->BeginTransaction(false);
            else
                EditorData->BeginTransaction();

            if (Destination->Parent)
            {
                Destination->Parent->Children.Remove(DragDrop->Destination);
                Destination->Parent->Children.Add(Destination);
            }
            else
            {
                EditorData->RootItems.Remove(DragDrop->Destination);
                EditorData->RootItems.Add(Destination);
            }

            if (Source)
                Source->Children.Remove(Target);
            else
                EditorData->RootItems.Remove(Target);

            Destination->Children.Add(DragDrop->Destination);
            Destination->Children.Add(Target);

            auto PrevParent = Destination->Parent;

            Target->Parent = Destination;
            DragDrop->Destination->Parent = Destination;

            //if (PrevParent)
            //    PrevParent->GenerateTableRow();
            //if (Source)
            //    Source->GenerateTableRow();
            EditorData->ItemExpansionChangedDelegate.ExecuteIfBound(Destination, true);
        }
        if (Source)
        {
            GenerateWidgetForItem(Source);
        }
    }
    EditorData->TreeStructureChangedDelegate.ExecuteIfBound();
    //EditorData->TreeWidget->RequestTreeRefresh();

    GEditor->EndTransaction();
    Destination->UpdateFolderIcon();
    GenerateWidgetForItem(Destination);

    auto Reply = FReply::Handled();
    Reply.EndDragDrop();

    return Reply;
}


void SLightHierarchyWidget::SearchBarOnChanged(const FText& NewString)
{
    SearchString = NewString.ToString();
    for (auto RootItem : EditorData->RootItems)
    {
        GetWidgetForItem(RootItem)->CheckNameAgainstSearchString(SearchString);
    }

    //Tree->RequestTreeRefresh();
    Tree->RebuildList();
}

TSharedPtr<STreeItemWidget> SLightHierarchyWidget::GetWidgetForItem(UItemHandle* ItemHandle)
{
    return ItemWidgets.FindOrAdd(ItemHandle);
}

void SLightHierarchyWidget::UpdateExpansionForItem(UItemHandle* ItemHandle, bool bContinueRecursively)
{
    check(ItemWidgets.Find(ItemHandle));
    Tree->SetItemExpansion(ItemHandle, ItemWidgets[ItemHandle]->bExpanded);

    if (bContinueRecursively)
    {
	    for (auto& Child : ItemHandle->Children)
	    {
            Tree->SetItemExpansion(Child, ItemWidgets[ItemHandle]->bExpanded);
	    }
    }
}

void SLightHierarchyWidget::ExpansionChangedCallback(UItemHandle* ItemHandle, bool bNewState)
{
    check(ItemWidgets.Find(ItemHandle));
    ItemWidgets[ItemHandle]->bExpanded = bNewState;

}

void SLightHierarchyWidget::ChangeExpansionInTree(UItemHandle* ItemHandle, bool bNewState)
{
    check(ItemWidgets.Find(ItemHandle));
    ItemWidgets[ItemHandle]->bExpanded = bNewState;
    Tree->SetItemExpansion(ItemHandle, bNewState);
}

FText SLightHierarchyWidget::GetPresetFilename() const
{
    if (EditorData->ToolPresetPath.IsEmpty())
    {
        return FText::FromString("Not Saved");
    }
    FString Path, Name, Extension;
    FPaths::Split(EditorData->ToolPresetPath, Path, Name, Extension);
    return FText::FromString(Name);
}

