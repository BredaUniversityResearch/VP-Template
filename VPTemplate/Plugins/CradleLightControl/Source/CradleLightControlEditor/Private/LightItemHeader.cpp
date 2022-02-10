#include "LightItemHeader.h"

#include "EditorData.h"
#include "ItemHandle.h"
#include "BaseLight.h"
#include "CradleLightControlEditor.h"

void SLightItemHeader::Construct(const FArguments& Args)
{
    check(Args._EditorData);
    EditorData = Args._EditorData;
    TreeHierarchyWidget = Args._TreeHierarchyWidget;
    ChildSlot
        .HAlign(HAlign_Fill)
        [
            SAssignNew(ContentBox, SBox)
        ];

    Update();

}

void SLightItemHeader::Update()
{
    if (EditorData->IsAMasterLightSelected() || EditorData->IsSingleGroupSelected())
    {
        SHorizontalBox::FSlot* NameSlot;
        SHorizontalBox::FSlot* CheckboxSlot;
        SVerticalBox::FSlot* TopSlot;
        TEnumAsByte<ELightType> IconType;
        if (EditorData->IsSingleGroupSelected())
        {
            IconType = Mixed;
        }
        else
        {
            IconType = EditorData->GetMasterLight()->Item->Type;
        }
        for (auto LightHandle : EditorData->GetSelectedLights())
        {
            if (IconType != LightHandle->Item->Type)
            {
                IconType = Mixed;
                break;
            }
        }
        LightHeaderCheckboxStyle = FCradleLightControlEditorModule::Get().MakeCheckboxStyleForType(IconType);

        ContentBox->SetHAlign(HAlign_Fill);
        ContentBox->SetPadding(FMargin(5.0f, 0.0f));
        ContentBox->SetContent(
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .Expose(TopSlot)
            .Padding(0.0f, 0.0f, 0.0f, 5.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
            .HAlign(HAlign_Fill)
            .Expose(NameSlot)
            [
                SAssignNew(LightHeaderNameBox, SBox)
                // SNew(STextBlock)
                // .Text(this, &SLightControlTool::LightHeaderText)
                // .Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 18))
            ]
        + SHorizontalBox::Slot()
            .HAlign(HAlign_Right)
            .Padding(0.0f, 0.0f, 15.0f, 0.0f)
            .Expose(CheckboxSlot)
            [
                SNew(SCheckBox)
                .Style(&LightHeaderCheckboxStyle)
            .OnCheckStateChanged(this, &SLightItemHeader::OnLightHeaderCheckStateChanged)
            .IsChecked(this, &SLightItemHeader::GetLightHeaderCheckState)
            .RenderTransform(FSlateRenderTransform(1.2f))
            ]
            ]
        + SVerticalBox::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Bottom)
            .Padding(0.0f, 3.0f, 30.0f, 3.0f)
            [
                SAssignNew(ExtraNoteBox, SBox)
            ]
        + SVerticalBox::Slot()
            .Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(this, &SLightItemHeader::LightHeaderExtraLightsText)
            .Visibility(EditorData->IsSingleGroupSelected()
                || EditorData->MultipleLightsInSelection() ?
                EVisibility::Visible : EVisibility::Collapsed)
            ]
        );
        bItemNoteChangeInProgress = false;
        bItemRenameInProgress = false;
        UpdateItemNameBox();
        UpdateExtraNoteBox();

        CheckboxSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
        TopSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    }
    else
    {
        ContentBox->SetContent(
            SNew(STextBlock)
            .Text(FText::FromString("No lights currently selected"))
            .Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 18)));
    }
}


void SLightItemHeader::OnLightHeaderCheckStateChanged(ECheckBoxState NewState)
{
    if (EditorData->IsAMasterLightSelected())
    {
        GEditor->BeginTransaction(FText::FromString(EditorData->GetMasterLight()->Name + " State Changed"));
        for (auto Light : EditorData->GetSelectedLights())
        {
            Light->Item->SetEnabled(NewState == ECheckBoxState::Checked); // Use the callback used by the tree to modify the state
        }
        GEditor->EndTransaction();
    }
}


ECheckBoxState SLightItemHeader::GetLightHeaderCheckState() const
{
    if (EditorData->IsAMasterLightSelected())
    {
        return EditorData->GetMasterLight()->Item->IsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }
    return ECheckBoxState::Undetermined;
}

FText SLightItemHeader::LightHeaderExtraLightsText() const
{
    if (EditorData->MultipleItemsSelected())
    {
        int GroupCount = 0;
        int LightCount = 0;
        int TotalLightCount = 0;
        for (auto SelectedItemHandle : EditorData->SelectedItems)
        {
            if (!SelectedItemHandle->Item)
            {
                GroupCount++;
            }
            else
            {
                LightCount++;
            }
            TotalLightCount += SelectedItemHandle->LightCount();
        }

        return FText::FromString(FString::Printf(TEXT("%d Groups and %d Lights selected (Total %d lights affected)"), GroupCount, LightCount, TotalLightCount));
    }
    return FText::FromString("");
}

void SLightItemHeader::UpdateItemNameBox()
{
    auto Item = EditorData->GetSingleSelectedItem();
    if (Item)
    {
        if (bItemRenameInProgress)
        {
            LightHeaderNameBox->SetContent(
                SNew(SEditableText)
                .Text(this, &SLightItemHeader::ItemNameText)
                .OnTextCommitted(this, &SLightItemHeader::CommitNewItemName)
                .SelectAllTextWhenFocused(true)
                .Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 18))
            );
        }
        else
        {
            LightHeaderNameBox->SetContent(
                SNew(STextBlock)
                .Text(this, &SLightItemHeader::ItemNameText)
                .OnDoubleClicked(this, &SLightItemHeader::StartItemNameChange)
                .Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 18))
            );
        }
    }
    TreeHierarchyWidget->Tree->RequestTreeRefresh();
}

FReply SLightItemHeader::StartItemNameChange(const FGeometry&, const FPointerEvent&)
{
    bItemRenameInProgress = true;
    UpdateItemNameBox();
    return FReply::Handled();
}

FText SLightItemHeader::ItemNameText() const
{
    if (EditorData->IsSingleGroupSelected())
    {
        return FText::FromString(EditorData->GetSelectedGroup()->Name);

    }
    return FText::FromString(EditorData->GetMasterLight()->Name);
}

void SLightItemHeader::CommitNewItemName(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter && !Text.IsEmpty())
    {
        auto Item = EditorData->GetSingleSelectedItem();
        GEditor->BeginTransaction(FText::FromString(Item->Name + " Rename"));
        Item->BeginTransaction();

        Item->Name = Text.ToString();
        TreeHierarchyWidget->GenerateWidgetForItem(Item);

        GEditor->EndTransaction();
    }
    bItemRenameInProgress = false;
    UpdateItemNameBox();
}

void SLightItemHeader::UpdateExtraNoteBox()
{
    ExtraNoteBox->SetVisibility(EVisibility::Visible);
    if (EditorData->IsSingleGroupSelected())
    {
        ExtraNoteBox->SetContent(
            SNew(STextBlock)
            .Text(FText::FromString("Group")));
    }
    else if (EditorData->IsAMasterLightSelected())
    {
        auto MasterLight = EditorData->GetMasterLight();
        if (MasterLight->Note.IsEmpty() && !bItemNoteChangeInProgress)
        {
            ExtraNoteBox->SetContent(
                SNew(STextBlock)
                .Text(FText::FromString("+ Add note"))
                .OnDoubleClicked(this, &SLightItemHeader::StartItemNoteChange)
            );
        }
        else if (bItemNoteChangeInProgress)
        {
            ExtraNoteBox->SetContent(
                SNew(SEditableText)
                .Text(this, &SLightItemHeader::ItemNoteText)
                .OnTextCommitted(this, &SLightItemHeader::CommitNewItemNote)
            );
        }
        else
        {
            ExtraNoteBox->SetContent(
                SNew(STextBlock)
                .Text(this, &SLightItemHeader::ItemNoteText)
                .OnDoubleClicked(this, &SLightItemHeader::StartItemNoteChange)
            );
        }
    }
    else
        ExtraNoteBox->SetVisibility(EVisibility::Collapsed);
}

FReply SLightItemHeader::StartItemNoteChange(const FGeometry&, const FPointerEvent&)
{
    bItemNoteChangeInProgress = true;
    UpdateExtraNoteBox();
    return FReply::Handled();
}

FText SLightItemHeader::ItemNoteText() const
{
    auto Item = EditorData->GetMasterLight();
    if (Item->Note.IsEmpty())
    {
        return FText::FromString("Note");
    }
    return FText::FromString(EditorData->GetMasterLight()->Note);
}

void SLightItemHeader::CommitNewItemNote(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        auto Item = EditorData->GetMasterLight();
        GEditor->BeginTransaction(FText::FromString(Item->Name + " Note changed"));
        Item->BeginTransaction();

        Item->Note = Text.ToString();
        //Item->GenerateTableRow();

        GEditor->EndTransaction();
    }
    bItemNoteChangeInProgress = false;
    UpdateExtraNoteBox();
}

