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
#include "ToolData.h"
#include "TreeItemWidget.h"

#include "LightControlTool.h"

#include "VirtualLight.h"

void SLightTreeHierarchy::Construct(const FArguments& Args)
{
    



    SaveIcon = FSlateIconFinder::FindIcon("AssetEditor.SaveAsset");
    SaveAsIcon = FSlateIconFinder::FindIcon("AssetEditor.SaveAssetAs");
    LoadIcon = FSlateIconFinder::FindIcon("EnvQueryEditor.Profiler.LoadStats");

    FSlateFontInfo Font24(FCoreStyle::GetDefaultFont(), 20);
    _ASSERT(Args._ToolData);
    ToolData = Args._ToolData;
    DataUpdateDelegate = Args._DataUpdateDelegate;
    DataVerificationDelegate = Args._DataVerificationDelegate;
    SelectionChangedDelegate = Args._SelectionChangedDelegate;
    HeaderText = FText::FromString(Args._Name);

    ToolData->OnToolDataLoaded = FOnToolDataLoadedDelegate::CreateRaw(this, &SLightTreeHierarchy::OnToolDataLoadedCallback);
    ToolData->LoadMetaData();
    ToolData->ItemExpansionChangedDelegate = FItemExpansionChangedDelegate::CreateLambda([this](UItemHandle* Item, bool bState)
        {
            GetWidgetForItem(Item)->bExpanded = bState;
            UpdateExpansionForItem(Item, false);
        });

    ToolData->TreeStructureChangedDelegate = FOnTreeStructureChangedDelegate::CreateLambda([this]()
        {
            ItemWidgets.Empty();
			for (auto& Item : ToolData->ListOfTreeItems)
			{
                ItemWidgets.FindOrAdd(Item) = SNew(STreeItemWidget, Item);
			}
    		Tree->RequestTreeRefresh();
        });
    if (DataVerificationDelegate.IsBound())
        LightVerificationTimer = RegisterActiveTimer(Args._DataVerificationInterval, FWidgetActiveTimerDelegate::CreateRaw(this, &SLightTreeHierarchy::VerifyLights));
    else
        LightVerificationTimer.Reset();

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
                //.OnSearch(this, &SLightTreeHierarchy::SearchBarSearch)
                .OnTextChanged(this, &SLightTreeHierarchy::SearchBarOnChanged)
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
                        .Text(this, &SLightTreeHierarchy::GetPresetFilename)
                        .Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 14))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Right)
                    .VAlign(VAlign_Center)
                    .Expose(SaveButtonSlot)
                    [
                        SNew(SButton)
                        .ButtonColorAndOpacity(FSlateColor(FColor::Transparent))
                        .OnClicked_UObject(ToolData, &UToolData::SaveCallBack)
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
                        .OnClicked_UObject(ToolData, &UToolData::SaveAsCallback)
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
                        .OnClicked_UObject(ToolData, &UToolData::LoadCallBack)
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
                .TreeItemsSource(&ToolData->RootItems)
                .OnSelectionChanged(this, &SLightTreeHierarchy::SelectionCallback)
                .OnGenerateRow(this, &SLightTreeHierarchy::AddToTree)
                .OnGetChildren(this, &SLightTreeHierarchy::GetTreeItemChildren)
                .OnExpansionChanged(this, &SLightTreeHierarchy::ExpansionChangedCallback)
                
            ]
        ]
        +SVerticalBox::Slot()
        .VAlign(VAlign_Bottom)
        .Padding(0.0f, 10.0f, 0.0f, 0.0f)
        .Expose(NewFolderButtonSlot)
        [
            SNew(SButton)
            .Text(FText::FromString("Add New Group"))
            .OnClicked_Raw(this, &SLightTreeHierarchy::AddFolderToTree)
        ]
    ];

    SaveButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    SaveAsButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    LoadButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    LightSearchBarSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    NewFolderButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

}

void SLightTreeHierarchy::PreDestroy()
{
    if (LightVerificationTimer)
        UnRegisterActiveTimer(LightVerificationTimer.ToSharedRef());
}

void SLightTreeHierarchy::OnActorSpawned(AActor* Actor)
{
    auto Type = Invalid;

    if (Cast<ASkyLight>(Actor))
        Type = ETreeItemType::SkyLight;
    else if (Cast<ASpotLight>(Actor))
        Type = ETreeItemType::SpotLight;
    else if (Cast<ADirectionalLight>(Actor))
        Type = ETreeItemType::DirectionalLight;
    else if (Cast<APointLight>(Actor))
        Type = ETreeItemType::PointLight;

    if (Type != Invalid)
    {
        auto NewItemHandle = ToolData->AddItem();
        NewItemHandle->Type = Type;
        NewItemHandle->Name = Actor->GetName();

        auto Item = Cast<UVirtualLight>(NewItemHandle->Item);

        switch (Type)
        {
        case SkyLight:
            Item->SkyLight = Cast<ASkyLight>(Actor);
            break;
        case SpotLight:
            Item->SpotLight = Cast<ASpotLight>(Actor);
            break;
        case DirectionalLight:
            Item->DirectionalLight = Cast<ADirectionalLight>(Actor);
            break;
        case PointLight:
            Item->PointLight = Cast<APointLight>(Actor);
            break;
        }
        DataUpdateDelegate.ExecuteIfBound(NewItemHandle);
        GetWidgetForItem(NewItemHandle)->CheckNameAgainstSearchString(SearchString);

        ToolData->RootItems.Add(NewItemHandle);
        GenerateWidgetForItem(NewItemHandle);

        Tree->RequestTreeRefresh();
    }
}

void SLightTreeHierarchy::BeginTransaction()
{
    ToolData->Modify();
}

void SLightTreeHierarchy::GenerateWidgetForItem(UItemHandle* Item)
{
    ItemWidgets.FindOrAdd(Item) = SNew(STreeItemWidget, Item);
}

TSharedRef<ITableRow> SLightTreeHierarchy::AddToTree(UItemHandle* ItemPtr,
                                                     const TSharedRef<STableViewBase>& OwnerTable)
{
    SHorizontalBox::FSlot& CheckBoxSlot = SHorizontalBox::Slot();
    CheckBoxSlot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    auto Widget = GetWidgetForItem(ItemPtr);

    if (!Widget)
    {
	    Widget = SNew(STreeItemWidget, ItemPtr);
    }
    
    auto Row =
        SNew(STableRow<UItemHandle*>, OwnerTable)
        .Padding(2.0f)
        .OnDragDetected_Raw(this, &SLightTreeHierarchy::DragDropBegin)
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

void SLightTreeHierarchy::GetTreeItemChildren(UItemHandle* Item, TArray<UItemHandle*>& Children)
{
    Children.Append(Item->Children);
}

void SLightTreeHierarchy::SelectionCallback(UItemHandle* Item, ESelectInfo::Type SelectType)
{
    // Strangely, this callback is triggered when an redo is done that brings back a deleted group
    // When that happens, the tree widget is considered to have no selected items, which will incorrectly make the masterlight a nullptr
    // In these cases, Item is garbage and invalid, so by verifying the validity we ensure we don't set the masterlight to a nullptr incorrectly. 
    if (IsValid(Item))
    {

        auto Objects = Tree->GetSelectedItems();
        auto& SelectedItems = ToolData->SelectedItems;
        auto& LightsUnderSelection = ToolData->LightsUnderSelection;
        auto& SelectionMasterLight = ToolData->SelectionMasterLight;
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

FReply SLightTreeHierarchy::AddFolderToTree()
{
    UItemHandle* NewFolder = ToolData->AddItem(true);
    NewFolder->Name = "New Group";
    GetWidgetForItem(NewFolder)->CheckNameAgainstSearchString(SearchString);
    ToolData->RootItems.Add(NewFolder);
    GenerateWidgetForItem(NewFolder);

    Tree->RequestTreeRefresh();


    return FReply::Handled();
}

void SLightTreeHierarchy::OnToolDataLoadedCallback(uint8 LoadingResult)
{
    if (LoadingResult == UItemHandle::ELoadingResult::Success)
    {
        UE_LOG(LogTemp, Display, TEXT("Light control state loaded successfully"));
    }
    else
    {
        FString ErrorMessage = "";

        switch (LoadingResult)
        {
        case UItemHandle::ELoadingResult::LightNotFound:
            ErrorMessage = "At least one light could not be found. Please ensure all lights exist and haven't been renamed since the w.";
            break;
        case UItemHandle::ELoadingResult::EngineError:
            ErrorMessage = "There was an error with the engine. Please try loading again. If the error persists, restart the engine.";
            break;
        case UItemHandle::ELoadingResult::InvalidType:
            ErrorMessage = "The item type that was tried to be loaded was not valid. Please ensure that the item type in the .json file is between 0 and 4.";
            break;
        case UItemHandle::ELoadingResult::MultipleErrors:
            ErrorMessage = "Multiple errors occurred. See output log for more details.";
            break;
        }

        UE_LOG(LogTemp, Display, TEXT("Light control state could not load with following message: %s"), *ErrorMessage);

        FNotificationInfo NotificationInfo(FText::FromString(FString::Printf(TEXT("Light control tool state could not be loaded. Please check the output log."))));

        NotificationInfo.ExpireDuration = 30.0f;
        NotificationInfo.bUseSuccessFailIcons = false;

        FSlateNotificationManager::Get().AddNotification(NotificationInfo);
    }

    for (auto& Item : ToolData->RootItems)
    {
        RegenerateItemHandleWidgets(Item);
    }
}

void SLightTreeHierarchy::RegenerateItemHandleWidgets(UItemHandle* ItemHandle)
{
    GenerateWidgetForItem(ItemHandle);

    for (auto Child : ItemHandle->Children)
    {
        RegenerateItemHandleWidgets(Child);
    }
}


EActiveTimerReturnType SLightTreeHierarchy::VerifyLights(double, float)
{
    DataVerificationDelegate.Execute();

    return EActiveTimerReturnType::Continue;
}

FReply SLightTreeHierarchy::DragDropBegin(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
    TSharedRef<FItemDragDropOperation> DragDropOp = MakeShared<FItemDragDropOperation>();
    DragDropOp->DraggedItems = ToolData->GetSelectedItems();
    
    FReply Reply = FReply::Handled();

    Reply.BeginDragDrop(DragDropOp);

    return Reply;
}

FReply SLightTreeHierarchy::DragDropEnd(const FDragDropEvent& DragDropEvent)
{
    auto DragDrop = StaticCastSharedPtr<FItemDragDropOperation>(DragDropEvent.GetOperation());
	UItemHandle* Destination = DragDrop->Destination;
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

        if (GEditor)
        {
            GEditor->BeginTransaction(FText::FromString("Light control tree drag and drop"));
        }

        // The source folder and the dragged item will always be affected, so always begin transacting them
        if (Source)
            Source->BeginTransaction(false);
        else
        {
            ToolData->BeginTransaction();
        }
        Target->BeginTransaction(false);
        Destination->BeginTransaction(false);
        if (Destination->Type == Folder)
        {

            Destination = DragDrop->Destination;

            Destination->BeginTransaction(false);


            if (Source)
                Source->Children.Remove(Target);
            else
                ToolData->RootItems.Remove(Target);
            Destination->Children.Add(Target);
            Target->Parent = Destination;

            /*if (Source)
                Source->GenerateTableRow();
            Destination->GenerateTableRow();*/
            ToolData->ItemExpansionChangedDelegate.ExecuteIfBound(Destination, true);
        }
        else
        {
            auto Parent = Destination->Parent;
            Destination = ToolData->AddItem(true);
            Destination->Name = DragDrop->Destination->Name + " Group";
            Destination->Parent = Parent;


            if (Destination->Parent)
                Destination->Parent->BeginTransaction(false);
            else
                ToolData->BeginTransaction();

            Destination->BeginTransaction(false);

            if (Destination->Parent)
            {
                Destination->Parent->Children.Remove(DragDrop->Destination);
                Destination->Parent->Children.Add(Destination);
            }
            else
            {
                ToolData->RootItems.Remove(DragDrop->Destination);
                ToolData->RootItems.Add(Destination);
            }

            if (Source)
                Source->Children.Remove(Target);
            else
                ToolData->RootItems.Remove(Target);

            Destination->Children.Add(DragDrop->Destination);
            Destination->Children.Add(Target);

            auto PrevParent = Destination->Parent;

            Target->Parent = Destination;
            DragDrop->Destination->Parent = Destination;

            //if (PrevParent)
            //    PrevParent->GenerateTableRow();
            //if (Source)
            //    Source->GenerateTableRow();
            ToolData->ItemExpansionChangedDelegate.ExecuteIfBound(Destination, true);
        }
        if (Source)
        {
            GenerateWidgetForItem(Source);
        }
    }
    ToolData->TreeStructureChangedDelegate.ExecuteIfBound();
    //ToolData->TreeWidget->RequestTreeRefresh();

    GEditor->EndTransaction();
    Destination->UpdateFolderIcon();
    GenerateWidgetForItem(Destination);

    auto Reply = FReply::Handled();
    Reply.EndDragDrop();

    return Reply;
}


void SLightTreeHierarchy::SearchBarOnChanged(const FText& NewString)
{
    SearchString = NewString.ToString();
    for (auto RootItem : ToolData->RootItems)
    {
        GetWidgetForItem(RootItem)->CheckNameAgainstSearchString(SearchString);
    }

    //Tree->RequestTreeRefresh();
    Tree->RebuildList();
}

TSharedPtr<STreeItemWidget> SLightTreeHierarchy::GetWidgetForItem(UItemHandle* ItemHandle)
{
    return ItemWidgets.FindOrAdd(ItemHandle);
}

void SLightTreeHierarchy::UpdateExpansionForItem(UItemHandle* ItemHandle, bool bContinueRecursively)
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

void SLightTreeHierarchy::ExpansionChangedCallback(UItemHandle* ItemHandle, bool bNewState)
{
    check(ItemWidgets.Find(ItemHandle));
    ItemWidgets[ItemHandle]->bExpanded = bNewState;

}

void SLightTreeHierarchy::ChangeExpansionInTree(UItemHandle* ItemHandle, bool bNewState)
{
    check(ItemWidgets.Find(ItemHandle));
    ItemWidgets[ItemHandle]->bExpanded = bNewState;
    Tree->SetItemExpansion(ItemHandle, bNewState);
}

FText SLightTreeHierarchy::GetPresetFilename() const
{
    if (ToolData->ToolPresetPath.IsEmpty())
    {
        return FText::FromString("Not Saved");
    }
    FString Path, Name, Extension;
    FPaths::Split(ToolData->ToolPresetPath, Path, Name, Extension);
    return FText::FromString(Name);
}

