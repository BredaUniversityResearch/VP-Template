#include "ItemHandle.h"

#include "BaseLight.h"
#include "ToolData.h"

#include "LightControlTool.h"

#include "Styling/SlateIconFinder.h"

ECheckBoxState UItemHandle::IsLightEnabled() const
{
    bool AllOff = true, AllOn = true;

    if (Type != Folder)
    {
        return Item->IsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }

    for (auto& Child : Children)
    {
        auto State = Child->IsLightEnabled();
        if (State == ECheckBoxState::Checked)
            AllOff = false;
        else if (State == ECheckBoxState::Unchecked)
            AllOn = false;
        else if (State == ECheckBoxState::Undetermined)
            return ECheckBoxState::Undetermined;

        if (!AllOff && !AllOn)
            return ECheckBoxState::Undetermined;
    }

    if (AllOn)
        return ECheckBoxState::Checked;
    else
        return ECheckBoxState::Unchecked;


}

void UItemHandle::OnCheck(ECheckBoxState NewState)
{
    bool B = false;
    if (NewState == ECheckBoxState::Checked)
        B = true;

    GEditor->BeginTransaction(FText::FromString(Name + " State change"));

    Item->SetEnabled(B);

    GEditor->EndTransaction();
}

FReply UItemHandle::TreeDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
    TSharedRef<FItemDragDropOperation> DragDropOp = MakeShared<FItemDragDropOperation>();
    DragDropOp->DraggedItem = this;

    FReply Reply = FReply::Handled();

    Reply.BeginDragDrop(DragDropOp);

    return Reply;
}

FReply UItemHandle::TreeDropDetected(const FDragDropEvent& DragDropEvent)
{
    auto DragDrop = StaticCastSharedPtr<FItemDragDropOperation>(DragDropEvent.GetOperation());
    auto Target = DragDrop->DraggedItem;
    auto Source = Target->Parent;
    UItemHandle* Destination = nullptr;

    if (!VerifyDragDrop(Target, this))
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
    BeginTransaction(false);
    if (Type == Folder)
    {

        Destination = this;

        Destination->BeginTransaction(false);


        if (Source)
            Source->Children.Remove(Target);
        else
            ToolData->RootItems.Remove(Target);
        Destination->Children.Add(Target);
        Target->Parent = Destination;

        if (Source)
            Source->GenerateTableRow();
        Destination->GenerateTableRow();
        ToolData->ItemExpansionChangedDelegate.ExecuteIfBound(Destination, true);
    }
    else
    {

        Destination = ToolData->AddItem(true);
        Destination->Name = Name + " Group";
        Destination->Parent = Parent;


        if (Parent)
            Parent->BeginTransaction(false);
        else
            ToolData->BeginTransaction();

        Destination->BeginTransaction(false);

        if (Parent)
        {
            Parent->Children.Remove(this);
            Parent->Children.Add(Destination);
        }
        else
        {
            ToolData->RootItems.Remove(this);
            ToolData->RootItems.Add(Destination);
        }

        if (Source)
            Source->Children.Remove(Target);
        else
            ToolData->RootItems.Remove(Target);

        Destination->Children.Add(Target);
        Destination->Children.Add(this);

        auto PrevParent = Parent;

        Target->Parent = Destination;
        Parent = Destination;

        if (PrevParent)
            PrevParent->GenerateTableRow();
        if (Source)
            Source->GenerateTableRow();

        ToolData->ItemExpansionChangedDelegate.ExecuteIfBound(Destination, true);
    }

    ToolData->TreeStructureChangedDelegate.ExecuteIfBound();
    //ToolData->TreeWidget->RequestTreeRefresh();

    GEditor->EndTransaction();
    Destination->UpdateFolderIcon();

    auto Reply = FReply::Handled();
    Reply.EndDragDrop();

    return FReply::Handled();
}

void UItemHandle::GenerateTableRow()
{
    if (!TableRowBox)
        return;
    auto IconType = Type;
    if (Type == Folder)
    {
        if (Children.Num())
        {
            IconType = Children[0]->Type; // This is 0 if there is a folder as the first child, which leads to out of bounds indexing
            for (size_t i = 1; i < Children.Num(); i++)
            {
                if (IconType != Children[i]->Type)
                {
                    IconType = Mixed;
                }
            }
        }
        else
            IconType = Mixed;
    }
    CheckBoxStyle = ToolData->MakeCheckboxStyleForType(IconType);

    CheckBoxStyle.CheckedPressedImage = CheckBoxStyle.UndeterminedImage;
    CheckBoxStyle.UncheckedPressedImage = CheckBoxStyle.UndeterminedImage;

    SHorizontalBox::FSlot* CheckBoxSlot;


    if (Type != Folder)
    {
        SHorizontalBox::FSlot* TextSlot;
        TableRowBox->SetContent(
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Expose(CheckBoxSlot) // On/Off toggle button 
            [
                SNew(SCheckBox)
                .IsChecked_UObject(this, &UItemHandle::IsLightEnabled)
            .OnCheckStateChanged_UObject(this, &UItemHandle::OnCheck)
            .Style(&CheckBoxStyle)
            ]
        + SHorizontalBox::Slot() // Name slot
            .Expose(TextSlot)
            .VAlign(VAlign_Center)
            [
                SAssignNew(RowNameBox, SBox)
            ]
        + SHorizontalBox::Slot()
            .Padding(10.0f, 0.0f, 0.0f, 3.0f)
            .VAlign(VAlign_Bottom)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Note))
            ]
        );

        TextSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    }
    else
    {
        SHorizontalBox::FSlot* FolderImageSlot;
        SHorizontalBox::FSlot* CloseButtonSlot;
        TableRowBox->SetContent(
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot() // Name slot
            .VAlign(VAlign_Center)
            [
                SAssignNew(RowNameBox, SBox)
            ]
        + SHorizontalBox::Slot()
            .Expose(CloseButtonSlot)
            .HAlign(HAlign_Right)
            [
                SNew(SButton)
                .Text(FText::FromString("Delete"))
            .OnClicked_UObject(this, &UItemHandle::RemoveFromTree)
            ]
        + SHorizontalBox::Slot() // On/Off toggle button
            .Expose(CheckBoxSlot)
            .HAlign(HAlign_Right)
            [
                SAssignNew(StateCheckbox, SCheckBox)
                .IsChecked_UObject(this, &UItemHandle::IsLightEnabled)
            .OnCheckStateChanged_UObject(this, &UItemHandle::OnCheck)
            .Style(&CheckBoxStyle)
            .RenderTransform(FSlateRenderTransform(FScale2D(1.1f)))
            ]
        + SHorizontalBox::Slot()
            .Expose(FolderImageSlot)
            .HAlign(HAlign_Right)
            .Padding(3.0f, 0.0f, 3.0f, 0.0f)
            [
                SNew(SButton)
                .ButtonColorAndOpacity(FSlateColor(FColor::Transparent))
            .OnClicked_Lambda([this]() {
            bExpanded = !bExpanded;
            ExpandInTree();
            return FReply::Handled();
                })
            [
                SNew(SImage) // Image overlay for the button
                .Image_Lambda([this]() {return &(bExpanded ? ToolData->GetIcon(FolderOpened) : ToolData->GetIcon(FolderClosed)); })
                    .RenderTransform(FSlateRenderTransform(FScale2D(1.1f)))
            ]
            ]
        );
        //TableRowBox->SetRenderTransform(FSlateRenderTransform(FScale2D(1.2f)));
        UpdateFolderIcon();

        FolderImageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
        CloseButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    }
    CheckBoxSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

    auto Font = FSlateFontInfo(FCoreStyle::GetDefaultFont(), 10);
    if (Type == Folder) // Slightly larger font for group items
        Font.Size = 12;

    if (bInRename)
    {
        RowNameBox->SetContent(
            SNew(SEditableText)
            .Text(FText::FromString(Name))
            .Font(Font)
            .OnTextChanged_Lambda([this](FText Input)
                {
                    Name = Input.ToString();
                })
            .OnTextCommitted_UObject(this, &UItemHandle::EndRename));

    }
    else
    {
        RowNameBox->SetContent(
            SNew(STextBlock)
            .Text(FText::FromString(Name))
            .Font(Font)
            .ShadowColorAndOpacity(FLinearColor::Blue)
            .ShadowOffset(FIntPoint(-1, 1))
            .OnDoubleClicked_UObject(this, &UItemHandle::StartRename));
    }

    if (bMatchesSearchString)
        TableRowBox->SetVisibility(EVisibility::Visible);
    else
        TableRowBox->SetVisibility(EVisibility::Collapsed);
}

bool UItemHandle::VerifyDragDrop(UItemHandle* Dragged, UItemHandle* Destination)
{
    // Would result in the child and parent creating a circle
    if (Dragged->Children.Find(Destination) != INDEX_NONE)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Cannot drag parent to child");
        return false;
    }

    // Can't drag the item on itself, can we now
    if (Dragged == Destination)
    {
        return false;
    }


    // Would cause a circular dependency between the tree items
    if (Dragged->HasAsIndirectChild(Destination))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Cannot drag parent to indirect child");
        return false;
    }

    // No need to do anything in this case
    if (Destination->Children.Find(Dragged) != INDEX_NONE)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Drag drop cancelled");
        return false;
    }

    return true;
}

bool UItemHandle::HasAsIndirectChild(UItemHandle* ItemHandle)
{
    if (Children.Find(ItemHandle) != INDEX_NONE)
        return true;

    for (auto TreeItem : Children)
    {
        if (TreeItem->HasAsIndirectChild(ItemHandle))
            return true;
    }

    return false;
}

FReply UItemHandle::StartRename(const FGeometry&, const FPointerEvent&)
{
    bInRename = true;
    GenerateTableRow();
    return FReply::Handled();
}


void UItemHandle::EndRename(const FText& Text, ETextCommit::Type CommitType)
{
    if (ETextCommit::Type::OnEnter == CommitType)
    {
        Name = Text.ToString();
    }


    bInRename = false;
    GenerateTableRow();
}

TSharedPtr<FJsonValue> UItemHandle::SaveToJson()
{
    TSharedPtr<FJsonObject> JsonItem = MakeShared<FJsonObject>();


    int ItemState = Item->bIsEnabled;

    JsonItem->SetStringField("Name", Name);
    JsonItem->SetStringField("Note", Note);
    JsonItem->SetNumberField("Type", Type);
    JsonItem->SetBoolField("Expanded", bExpanded);
    if (Type != Folder)
    {
        JsonItem->SetObjectField("Item", Item->SaveAsJson());/*
        Item->SetStringField("RelatedLightName", SkyLight->GetName());
        Item->SetNumberField("State", ItemState);
        Item->SetNumberField("Intensity", Intensity);
        Item->SetNumberField("Hue", Hue);
        Item->SetNumberField("Saturation", Saturation);
        Item->SetBoolField("UseTemperature", bUseTemperature);
        Item->SetNumberField("Temperature", Temperature);*/
    }
    else
    {
        TArray<TSharedPtr<FJsonValue>> ChildrenJson;

        for (auto Child : Children)
        {
            ChildrenJson.Add(Child->SaveToJson());
        }

        JsonItem->SetArrayField("Children", ChildrenJson);

    }

    TSharedPtr<FJsonValue> JsonValue = MakeShared<FJsonValueObject>(JsonItem);
    return JsonValue;
}


UItemHandle::ELoadingResult UItemHandle::LoadFromJson(TSharedPtr<FJsonObject> JsonObject)
{
    Name = JsonObject->GetStringField("Name");
    Note = JsonObject->GetStringField("Note");
    Type = StaticCast<ETreeItemType>(JsonObject->GetNumberField("Type"));
    bExpanded = JsonObject->GetBoolField("Expanded");
    auto JsonItem = JsonObject->GetObjectField("Item");
    if (Type != Folder)
    {
        if (!GWorld)
        {
            UE_LOG(LogTemp, Error, TEXT("There was an error with the engine. Try loading again. If the issue persists, restart the engine."));
            return EngineError;
        }

        Item->LoadFromJson(JsonItem);
    }
    else
    {
        auto JsonChildren = JsonObject->GetArrayField("Children");

        auto ChildrenLoadingSuccess = Success;
        for (auto Child : JsonChildren)
        {
            const TSharedPtr<FJsonObject>* ChildObjectPtr;
            auto Success = Child->TryGetObject(ChildObjectPtr);
            auto ChildObject = *ChildObjectPtr;
            _ASSERT(Success);
            int ChildType = ChildObject->GetNumberField("Type");
            auto ChildItem = ToolData->AddItem(ChildType == 0);

            ChildItem->Parent = this;

            auto ChildResult = ChildItem->LoadFromJson(ChildObject);
            if (ChildResult != ELoadingResult::Success)
            {
                if (ChildrenLoadingSuccess == ELoadingResult::Success)
                {
                    ChildrenLoadingSuccess = ChildResult;
                }
                else
                    ChildrenLoadingSuccess = ELoadingResult::MultipleErrors;
            }
            Children.Add(ChildItem);
        }
        return ChildrenLoadingSuccess;
    }


    return Success;
}

void UItemHandle::ExpandInTree()
{
    ToolData->ItemExpansionChangedDelegate.ExecuteIfBound(this, bExpanded);

    for (auto Child : Children)
    {
        Child->ExpandInTree();
    }
}

FReply UItemHandle::RemoveFromTree()
{
    GEditor->BeginTransaction(FText::FromString("Delete Light control folder"));
    BeginTransaction(false);
    if (Parent)
    {
        Parent->BeginTransaction(false);
        for (auto Child : Children)
        {
            Child->BeginTransaction(false);
            Child->Parent = Parent;
            Parent->Children.Add(Child);

        }
        Parent->Children.Remove(this);
    }
    else
    {
        ToolData->BeginTransaction();
        for (auto Child : Children)
        {
            Child->BeginTransaction(false);
            Child->Parent = nullptr;
            ToolData->RootItems.Add(Child);
        }
        ToolData->RootItems.Remove(this);
    }
    GEditor->EndTransaction();
    Children.Empty();
    ToolData->TreeStructureChangedDelegate.ExecuteIfBound();

    return FReply::Handled();
}

void UItemHandle::GetLights(TArray<UItemHandle*>& Array)
{
    if (Type == Folder)
    {
        for (auto& Child : Children)
            Child->GetLights(Array);
    }
    else
    {
        Array.Add(this);
    }
}

void UItemHandle::UpdateFolderIcon()
{
    if (Type != Folder)
        return;
    TArray<UItemHandle*> ChildLights;
    GetLights(ChildLights);

    auto IconType = Type;

    if (ChildLights.Num() > 0)
    {
        IconType = ChildLights[0]->Type;

        for (size_t i = 1; i < ChildLights.Num(); i++)
        {
            if (IconType != ChildLights[i]->Type)
            {
                IconType = Mixed;
                break;
            }
        }
    }
    else
        IconType = Mixed;


    if (Parent)
        Parent->UpdateFolderIcon();
}

bool UItemHandle::CheckNameAgainstSearchString(const FString& SearchString)
{
    bMatchesSearchString = false;
    if (SearchString.Len() == 0)
    {
        bMatchesSearchString = true;
    }
    else if (Name.Find(SearchString) != -1)
    {
        bMatchesSearchString = true;
    }

    for (auto ChildItem : Children)
    {
        bMatchesSearchString |= ChildItem->CheckNameAgainstSearchString(SearchString);
    }

    return bMatchesSearchString;
}

int UItemHandle::LightCount() const
{
    if (Type != Folder)
    {
        return 1;
    }
    auto LightCount = 0;

    for (auto Child : Children)
    {
        LightCount += Child->LightCount();
    }

    return LightCount;
}

void UItemHandle::BeginTransaction(bool bAffectItem, bool bAffectParent)
{
    Modify();
    if (bAffectItem)
    {
        Item->BeginTransaction();
    }

    if (bAffectParent && Parent)
    {
        Parent->BeginTransaction(false);
    }
}

void UItemHandle::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
    UObject::PostTransacted(TransactionEvent);
    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
    {
        if (Type == Folder)
            ToolData->TreeStructureChangedDelegate.ExecuteIfBound();

        GenerateTableRow();
    }
}
