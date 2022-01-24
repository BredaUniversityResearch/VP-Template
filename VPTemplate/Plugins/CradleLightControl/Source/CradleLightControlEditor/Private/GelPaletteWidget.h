#pragma once

#include "Slate.h"

DECLARE_DELEGATE_OneParam(FGelPaletteSelectionCallback, const FLinearColor&)

class SGelPaletteWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SGelPaletteWidget) {}

	

	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	
	void ParseGelColorFile(FString Filepath);

	TSharedRef<ITableRow> GenerateTile(TSharedPtr<FColor> Item, const TSharedRef<STableViewBase>& Table);
	void OnSelectionMade(TSharedPtr<FColor> SelectedItem, ESelectInfo::Type SelectionType);

	void OnItemDoubleClicked(TSharedPtr<FColor> SelectedItem);
	
	TSharedPtr<FSlateColorBrush> WhiteBrush;

	TArray<TSharedPtr<FColor>> Items;
	FGelPaletteSelectionCallback SelectionCallback;
	TSharedPtr<SWindow> Window;
};