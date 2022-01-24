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

#include "LightControlTool.h"

#include "ToolData.h"
#include "VirtualLight.h"

#pragma region TreeItemStruct

//
//ECheckBoxState ULightTreeItem::IsLightEnabled() const
//{
//    bool AllOff = true, AllOn = true;
//
//    if (Type != Folder && !IsValid(SkyLight))
//        return ECheckBoxState::Undetermined;
//
//    switch (Type)
//    {
//    case ETreeItemType::SkyLight:
//        return SkyLight->GetLightComponent()->IsVisible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
//    case ETreeItemType::SpotLight:
//        return SpotLight->GetLightComponent()->IsVisible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
//    case ETreeItemType::DirectionalLight:
//        return DirectionalLight->GetLightComponent()->IsVisible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
//    case ETreeItemType::PointLight:
//        return PointLight->GetLightComponent()->IsVisible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
//    case ETreeItemType::Folder:
//        for (auto& Child : Children)
//        {
//            auto State = Child->IsLightEnabled();
//            if (State == ECheckBoxState::Checked)
//                AllOff = false;
//            else if (State == ECheckBoxState::Unchecked)
//                AllOn = false;
//            else if (State == ECheckBoxState::Undetermined)
//                return ECheckBoxState::Undetermined;
//
//            if (!AllOff && !AllOn)
//                return ECheckBoxState::Undetermined;
//        }
//
//        if (AllOn)
//            return ECheckBoxState::Checked;
//        else
//            return ECheckBoxState::Unchecked;
//
//
//    default:
//        return ECheckBoxState::Undetermined;
//    }
//}
//
//void ULightTreeItem::OnCheck(ECheckBoxState NewState)
//{
//    bool B = false;
//    if (NewState == ECheckBoxState::Checked)
//        B = true;
//    if (Type != Folder && !IsValid(ActorPtr))
//        return;
//
//    GEditor->BeginTransaction(FText::FromString(Name + " State change"));
//
//    SetEnabled(B);
//
//    GEditor->EndTransaction();
//}
//
//
//void ULightTreeItem::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
//{
//    UObject::PostTransacted(TransactionEvent);
//    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
//    {
//        if (Type != Folder)
//            OwningWidget->CoreToolPtr->GetLightPropertyEditor().Pin()->UpdateSaturationGradient(OwningWidget->TransactionalVariables->SelectionMasterLight->Hue);
//        else
//           OwningWidget->Tree->RequestTreeRefresh();
//
//        GenerateTableRow();
//    }
//}
//
//
//void ULightTreeItem::FetchDataFromLight()
//{
//    _ASSERT(Type != Folder);
//
//    FLinearColor RGB;
//
//    Intensity = 0.0f;
//    Saturation = 0.0f;
//    Temperature = 0.0f;
//
//    if (Type == ETreeItemType::SkyLight)
//    {
//        RGB = SkyLight->GetLightComponent()->GetLightColor();
//
//    }
//    else
//    {
//        ALight* LightPtr = Cast<ALight>(PointLight);
//        RGB = LightPtr->GetLightColor();
//    }
//    auto HSV = RGB.LinearRGBToHSV();
//    Saturation = HSV.G;
//
//    // If Saturation is 0, the color is white. The RGB => HSV conversion calculates the Hue to be 0 in that case, even if it's not supposed to be.
//    // Do this to preserve the Hue previously used rather than it getting reset to 0.
//    if (Saturation != 0.0f)
//        Hue = HSV.R;
//
//    if (Type == ETreeItemType::PointLight)
//    {
//        auto Comp = PointLight->PointLightComponent;
//        Intensity = Comp->Intensity;       
//    }    
//    else if (Type == ETreeItemType::SpotLight)
//    {
//        auto Comp = SpotLight->SpotLightComponent;
//        Intensity = Comp->Intensity;
//    }
//
//    if (Type != ETreeItemType::SkyLight)
//    {
//        auto LightPtr = Cast<ALight>(ActorPtr);
//        auto LightComp = LightPtr->GetLightComponent();
//        bUseTemperature = LightComp->bUseTemperature;
//        Temperature = LightComp->Temperature;
//
//        bCastShadows = LightComp->CastShadows;
//    }
//    else
//    {
//        bCastShadows = SkyLight->GetLightComponent()->CastShadows;
//    }
//
//    auto CurrentFwd = FQuat::MakeFromEuler(FVector(0.0f, Vertical, Horizontal)).GetForwardVector();
//    auto ActorQuat = ActorPtr->GetTransform().GetRotation().GetNormalized();
//    auto ActorFwd = ActorQuat.GetForwardVector();
//
//    if (CurrentFwd.Equals(ActorFwd))
//    {
//        auto Euler = ActorQuat.Euler();
//        Horizontal = Euler.Z;
//        Vertical = Euler.Y;
//    }
//    
//
//    if (Type == ETreeItemType::SpotLight)
//    {
//        InnerAngle = SpotLight->SpotLightComponent->InnerConeAngle;
//        OuterAngle = SpotLight->SpotLightComponent->OuterConeAngle;
//    }
//    UpdateDMX();
//}
//
//void ULightTreeItem::UpdateLightColor()
//{
//    auto NewColor = FLinearColor::MakeFromHSV8(StaticCast<uint8>(Hue / 360.0f * 255.0f), StaticCast<uint8>(Saturation * 255.0f), 255);
//    UpdateLightColor(NewColor);
//}
//
//void ULightTreeItem::UpdateLightColor(FLinearColor& Color)
//{
//    if (Type == ETreeItemType::SkyLight)
//    {
//        SkyLight->GetLightComponent()->SetLightColor(Color);
//        SkyLight->GetLightComponent()->UpdateLightSpriteTexture();
//    }
//    else
//    {
//        auto LightPtr = Cast<ALight>(PointLight);
//        LightPtr->SetLightColor(Color);
//        LightPtr->GetLightComponent()->UpdateLightSpriteTexture();
//    }
//    UpdateDMX();
//}
//
//void ULightTreeItem::UpdateDMX()
//{
//    if (DMXProperties.bUseDmx && DMXProperties.OutputPort && DMXProperties.DataConverter)
//    {
//        DMXProperties.DataConverter->Channels.Empty();
//        DMXProperties.DataConverter->StartingChannel = DMXProperties.StartingChannel;
//        DMXProperties.DataConverter->Convert(this);
//
//        //auto& Channels = DMXProperties.Channels;
//        //auto Start = DMXProperties.StartingChannel;
//
//        DMXProperties.OutputPort->SendDMX(1, DMXProperties.DataConverter->Channels);
//    }
//}
//



#pragma endregion
//
//void UTreeTransactionalVariables::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
//{
//    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo && Widget.IsValid())
//    {
//        Widget.Pin()->Tree->RequestTreeRefresh();
//    }
//}

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
            Tree->SetItemExpansion(Item, bState);
        });

    ToolData->TreeStructureChangedDelegate = FOnTreeStructureChangedDelegate::CreateLambda([this]()
        {
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
                .OnExpansionChanged(this, &SLightTreeHierarchy::TreeExpansionCallback)
                
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
        NewItemHandle->CheckNameAgainstSearchString(SearchString);

        ToolData->RootItems.Add(NewItemHandle);
        FCradleLightControlEditorModule::Get().GenerateItemHandleWidget(NewItemHandle);

        Tree->RequestTreeRefresh();
    }
}

void SLightTreeHierarchy::BeginTransaction()
{
    ToolData->Modify();
}

TSharedRef<ITableRow> SLightTreeHierarchy::AddToTree(UItemHandle* ItemPtr,
                                                     const TSharedRef<STableViewBase>& OwnerTable)
{
    SHorizontalBox::FSlot& CheckBoxSlot = SHorizontalBox::Slot();
    CheckBoxSlot.SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


    if (!ItemPtr->Type == Folder || ItemPtr->Children.Num() > 0)
    {
        CheckBoxSlot
        [
            SAssignNew(ItemPtr->StateCheckbox, SCheckBox)
                .IsChecked_UObject(ItemPtr, &UItemHandle::IsLightEnabled)
                .OnCheckStateChanged_UObject(ItemPtr, &UItemHandle::OnCheck)
        ];
    }
    if (!ItemPtr->TableRowBox)
    {
        SAssignNew(ItemPtr->TableRowBox, SBox);
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
        .Visibility_Lambda([ItemPtr]() {return ItemPtr->bMatchesSearchString ? EVisibility::Visible : EVisibility::Collapsed; })
        [
            ItemPtr->TableRowBox.ToSharedRef()
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
    NewFolder->CheckNameAgainstSearchString(SearchString);
    ToolData->RootItems.Add(NewFolder);
    FCradleLightControlEditorModule::Get().GenerateItemHandleWidget(NewFolder);

    Tree->RequestTreeRefresh();


    return FReply::Handled();
}

void SLightTreeHierarchy::TreeExpansionCallback(UItemHandle* Item, bool bExpanded)
{
    Item->bExpanded = bExpanded;
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
    FCradleLightControlEditorModule::Get().GenerateItemHandleWidget(ItemHandle);

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
            FCradleLightControlEditorModule::Get().GenerateItemHandleWidget(Source);
        }
    }
    ToolData->TreeStructureChangedDelegate.ExecuteIfBound();
    //ToolData->TreeWidget->RequestTreeRefresh();

    GEditor->EndTransaction();
    Destination->UpdateFolderIcon();
    FCradleLightControlEditorModule::Get().GenerateItemHandleWidget(Destination);

    auto Reply = FReply::Handled();
    Reply.EndDragDrop();

    return Reply;
}


void SLightTreeHierarchy::SearchBarOnChanged(const FText& NewString)
{
    SearchString = NewString.ToString();
    for (auto RootItem : ToolData->RootItems)
    {
        RootItem->CheckNameAgainstSearchString(SearchString);
    }

    //Tree->RequestTreeRefresh();
    Tree->RebuildList();
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

