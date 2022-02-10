#pragma once

#include "Slate.h"

class STreeItemWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(STreeItemWidget){}

	SLATE_END_ARGS();

    STreeItemWidget()
	    : bExpanded(false)
        , bMatchesSearchString(true)
		, bInRename(false)
    {}

	void Construct(const FArguments Args, class UItemHandle* ItemHandle, class UEditorData* EditorData);

    void OnCheck(ECheckBoxState NewState);

    FReply StartRename(const FGeometry&, const FPointerEvent&);
    void EndRename(const FText& Text, ETextCommit::Type CommitType);

    void ExpandInTree();

    // Check if the name of the handle matches the search string. Not case sensitive.
    bool CheckNameAgainstSearchString(const FString& SearchString);


    FReply RemoveFromTreeButtonClicked();

    void UpdateNameBox();
    FText GetItemName() const;

    class UEditorData* EditorData;

    UItemHandle* ItemHandleRef;
    class SLightHierarchyWidget* OwningWidgetRef;

    // Top widget which contains all other widgets for the handle's widget
    TSharedPtr<SBox> TableRowBox;

    TSharedPtr<SCheckBox> StateCheckbox;
    // SBox containing the widget for the name of the handle, or editable text if it is being renamed
    TSharedPtr<SBox> RowNameBox;
    FCheckBoxStyle CheckBoxStyle;


    bool bExpanded;
    bool bMatchesSearchString;

    bool bInRename;
};