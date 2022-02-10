#include "TreeItemWidget.h"

#include "BaseLight.h"
#include "EditorData.h"
#include "ItemHandle.h"
#include "CradleLightControlEditor.h"
#include "LightTreeHierarchy.h"
#include "ToolData.h"

void STreeItemWidget::Construct(const FArguments Args, UItemHandle* ItemHandle, UEditorData* InEditorData)
{
	EditorData = InEditorData;
	ItemHandleRef = ItemHandle;
	// Create a root widget for the Item Handle if one doesn't already exist
	// Determine what icon to use for the checkbox which is turns the light on/off
	// This is primarily for group items which may contain multiple types of lights
	TEnumAsByte<ELightType> IconType;
	if (!ItemHandleRef->Item)
	{
		if (ItemHandleRef->Children.Num())
		{
			// TODO: What if we have a group which contains, say, a point light and a group of point lights?
			// Will the icon be mixed or point light? Need to test.
			IconType = ItemHandleRef->Children[0]->Item->Type;
			for (size_t i = 1; i < ItemHandleRef->Children.Num(); i++)
			{
				if (IconType != ItemHandleRef->Children[i]->Item->Type)
				{
					IconType = Mixed;
				}
			}
		}
		else
			IconType = Mixed;
	}
	else
		IconType = ItemHandleRef->Item->Type;

	CheckBoxStyle = FCradleLightControlEditorModule::Get().MakeCheckboxStyleForType(IconType);
	CheckBoxStyle.CheckedPressedImage = CheckBoxStyle.UndeterminedImage;
	CheckBoxStyle.UncheckedPressedImage = CheckBoxStyle.UndeterminedImage;

	// The checkbox slot will be exposed with the goal of changing its size rule
	SHorizontalBox::FSlot* CheckBoxSlot;

		// The groups and light items have different widget layours, so we generate them differently based on type
	if (ItemHandleRef->Item)
	{
		// Generation of widget for light items

		// Exposed to change the size rule
		SHorizontalBox::FSlot* TextSlot;
		ChildSlot[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Expose(CheckBoxSlot) // On/Off toggle button 
			[
				SNew(SCheckBox)
				.IsChecked_UObject(ItemHandleRef, &UItemHandle::IsLightEnabled)
			.OnCheckStateChanged_Raw(this, &STreeItemWidget::OnCheck)
			.Style(&CheckBoxStyle)
			]
			+ SHorizontalBox::Slot() // Name slot
			.Expose(TextSlot)
			.VAlign(VAlign_Center)
			[
				SAssignNew(RowNameBox, SBox)
			]
			+ SHorizontalBox::Slot() // Additional note
			.Padding(10.0f, 0.0f, 0.0f, 3.0f)
			.VAlign(VAlign_Bottom)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ItemHandleRef->Note))
			]
		];

		TextSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
	}
	else
	{
		// Generation of group widget

		SHorizontalBox::FSlot* FolderImageSlot;
		SHorizontalBox::FSlot* CloseButtonSlot;
		ChildSlot[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot() // Name slot
			.VAlign(VAlign_Center)
			[
				SAssignNew(RowNameBox, SBox)
			]
			+ SHorizontalBox::Slot()
			.Expose(CloseButtonSlot) // Delete button slot
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(FText::FromString("Delete"))
				.OnClicked_Raw(this, &STreeItemWidget::RemoveFromTreeButtonClicked)
			]
			+ SHorizontalBox::Slot() // On/Off toggle button
			.Expose(CheckBoxSlot)
			.HAlign(HAlign_Right)
			[
				SAssignNew(StateCheckbox, SCheckBox)
				.IsChecked_UObject(ItemHandleRef, &UItemHandle::IsLightEnabled)
				.OnCheckStateChanged_Raw(this, &STreeItemWidget::OnCheck)
				.Style(&CheckBoxStyle)
				.RenderTransform(FSlateRenderTransform(FScale2D(1.1f)))
			]
			+ SHorizontalBox::Slot()
			.Expose(FolderImageSlot) // Additional expand/shrink button slot
			.HAlign(HAlign_Right)
			.Padding(3.0f, 0.0f, 3.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(FSlateColor(FColor::Transparent))
				.OnClicked_Lambda([this]()
				{
					bExpanded = !bExpanded;
					ExpandInTree();
					return FReply::Handled();
				})
				[
					SNew(SImage) // Image overlay for the button
					.Image_Lambda([this]() {return &(bExpanded ? FCradleLightControlEditorModule::Get().GetIcon(FolderOpened) : FCradleLightControlEditorModule::Get().GetIcon(FolderClosed)); })
					.RenderTransform(FSlateRenderTransform(FScale2D(1.1f)))
				]
			]
		];
		// TODO: Test if this even does anything.
		ItemHandleRef->UpdateFolderIcon();

		FolderImageSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
		CloseButtonSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;

		if (ItemHandleRef->Children.Num() < 1)
		{
			StateCheckbox->SetVisibility(EVisibility::Hidden);
		}
	}
	CheckBoxSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


	// Fill in the name slot differently based on whether the item is being renamed or not
	UpdateNameBox();
	

	// If the search string used in the tool does not match the name of the item, we hide it
	if (bMatchesSearchString)
		SetVisibility(EVisibility::Visible);
	else
		SetVisibility(EVisibility::Collapsed);
}

void STreeItemWidget::OnCheck(ECheckBoxState NewState)
{
	bool B = false;
	if (NewState == ECheckBoxState::Checked)
		B = true;
	GEditor->BeginTransaction(FText::FromString(ItemHandleRef->Name + " State change"));

	ItemHandleRef->EnableGrouped(B);

	GEditor->EndTransaction();
}


FReply STreeItemWidget::StartRename(const FGeometry&, const FPointerEvent&)
{
	bInRename = true;
	UpdateNameBox();
	return FReply::Handled();
}


void STreeItemWidget::EndRename(const FText& Text, ETextCommit::Type CommitType)
{
	if (ETextCommit::Type::OnEnter == CommitType)
	{
		GEditor->BeginTransaction(FText::FromString("Light control item rename"));
		ItemHandleRef->BeginTransaction(false);
		ItemHandleRef->Name = Text.ToString();
		GEditor->EndTransaction();
	}
	UpdateNameBox();

	bInRename = false;
}

void STreeItemWidget::ExpandInTree()
{
	OwningWidgetRef->UpdateExpansionForItem(ItemHandleRef);
	//ItemHandleRef->EditorData->ItemExpansionChangedDelegate.ExecuteIfBound(this, bExpanded);

	for (auto Child : ItemHandleRef->Children)
	{
		OwningWidgetRef->GetWidgetForItem(Child)->ExpandInTree();
	}
}

bool STreeItemWidget::CheckNameAgainstSearchString(const FString& SearchString)
{
	bMatchesSearchString = false;
	if (SearchString.Len() == 0)
	{
		bMatchesSearchString = true;
	}
	else if (ItemHandleRef->Name.Find(SearchString) != -1)
	{
		bMatchesSearchString = true;
	}

	for (auto ChildItem : ItemHandleRef->Children)
	{
		bMatchesSearchString |= OwningWidgetRef->GetWidgetForItem(ItemHandleRef)->CheckNameAgainstSearchString(SearchString);
	}

	return bMatchesSearchString;
}

FReply STreeItemWidget::RemoveFromTreeButtonClicked()
{
	GEditor->BeginTransaction(FText::FromString("Delete Light control folder"));

	ItemHandleRef->BeginTransaction(false);
	if (ItemHandleRef->Parent)
	{
		// If this handle has a parent, we move all of its children to the parent
		ItemHandleRef->Parent->BeginTransaction(false);
		for (auto Child : ItemHandleRef->Children)
		{
			Child->BeginTransaction(false);
			Child->Parent = ItemHandleRef->Parent;
			ItemHandleRef->Parent->Children.Add(Child);

		}
		ItemHandleRef->Parent->Children.Remove(ItemHandleRef);
	}
	else
	{
		// If the handle is a root item, we make its children root items as well
		ItemHandleRef->EditorData->BeginTransaction();
		for (auto Child : ItemHandleRef->Children)
		{
			Child->BeginTransaction(false);
			Child->Parent = nullptr;
			EditorData->RootItems.Add(Child);
		}
		EditorData->RootItems.Remove(ItemHandleRef);
	}
	GEditor->EndTransaction();

	ItemHandleRef->Children.Empty();
	ItemHandleRef->EditorData->TreeStructureChangedDelegate.ExecuteIfBound();

	return FReply::Handled();
}

void STreeItemWidget::UpdateNameBox()
{
	auto Font = FSlateFontInfo(FCoreStyle::GetDefaultFont(), 10);
	if (!ItemHandleRef->Item) // Slightly larger font for group items
		Font.Size = 12;
	if (bInRename)
	{
		RowNameBox->SetContent(
			SNew(SEditableText)
			.Text(FText::FromString(ItemHandleRef->Name))
			.Font(Font)
			.OnTextChanged_Lambda([this](FText Input)
				{
					//ItemHandleRef->Name = Input.ToString();
				})
			.OnTextCommitted(this, &STreeItemWidget::EndRename));

	}
	else
	{
		RowNameBox->SetContent(
			SNew(STextBlock)
			.Text(this, &STreeItemWidget::GetItemName)
			.Font(Font)
			.ShadowColorAndOpacity(FLinearColor::Blue)
			.ShadowOffset(FIntPoint(-1, 1))
			.OnDoubleClicked(this, &STreeItemWidget::StartRename));
	}
}

FText STreeItemWidget::GetItemName() const
{
	return FText::FromString(ItemHandleRef->Name);
}
