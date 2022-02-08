#pragma once
#include "Slate.h"

class UToolData;
class SLightItemHeader : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLightItemHeader) {}

    SLATE_ARGUMENT(class UEditorData*, EditorData)

    SLATE_ARGUMENT(TSharedPtr<class SLightHierarchyWidget>, TreeHierarchyWidget)

    SLATE_END_ARGS()

	void Construct(const FArguments& Args);

    void Update();


    void OnLightHeaderCheckStateChanged(ECheckBoxState NewState);
    ECheckBoxState GetLightHeaderCheckState() const;
    FText LightHeaderExtraLightsText() const;

    void UpdateItemNameBox();
    FReply StartItemNameChange(const FGeometry&, const FPointerEvent&); // called on text doubleclick
    FText ItemNameText() const;
    void CommitNewItemName(const FText& Text, ETextCommit::Type CommitType);


    void UpdateExtraNoteBox();
    FReply StartItemNoteChange(const FGeometry&, const FPointerEvent&);
    FText ItemNoteText() const;
    void CommitNewItemNote(const FText& Text, ETextCommit::Type CommitType);

    UEditorData* EditorData;
    TSharedPtr<class SLightHierarchyWidget> TreeHierarchyWidget;

    TSharedPtr<SBox> ContentBox;
    FCheckBoxStyle LightHeaderCheckboxStyle;


    TSharedPtr<SBox> ExtraNoteBox;
    TSharedPtr<SBox> LightHeaderNameBox;
    TSharedPtr<SBox> LightHeaderBox;

    bool bItemRenameInProgress;
    bool bItemNoteChangeInProgress;


};