#include "GelPaletteWidget.h"
#include "Interfaces/IPluginManager.h"
#include "CradleLightControl.h"

void SGelPaletteWidget::Construct(const FArguments& Args)
{
	auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
	auto Filepath = ThisPlugin->GetContentDir() + "/GelColors.txt";
	ParseGelColorFile(Filepath);


	WhiteBrush = MakeShared<FSlateColorBrush>(FColor::White);
	WhiteBrush->SetImageSize(FVector2D(16.0f, 16.0f));

	ChildSlot[
		SNew(STileView<TSharedPtr<FColor>>)
			.ListItemsSource(&Items)
			.ItemHeight(60.0f)
			.ItemWidth(40.0f)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateTile(this, &SGelPaletteWidget::GenerateTile)
			.OnSelectionChanged(this, &SGelPaletteWidget::OnSelectionMade)
			.OnMouseButtonDoubleClick(this, &SGelPaletteWidget::OnItemDoubleClicked)
	];
}

void SGelPaletteWidget::ParseGelColorFile(FString Filepath)
{
	FString Output;
	FFileHelper::LoadFileToString(Output, *Filepath);

	TArray<FString> ColorStrings;

	Output.ParseIntoArray(ColorStrings, *FString(";\r\n"));

	for(auto& S : ColorStrings)
	{
		auto Col = MakeShared<FColor>();
		TArray<FString> ChannelStrings;
		S.ParseIntoArray(ChannelStrings, *FString(","));

		Col->R = FCString::Atoi(*ChannelStrings[0]);
		Col->G = FCString::Atoi(*ChannelStrings[1]);
		Col->B = FCString::Atoi(*ChannelStrings[2]);
		Col->A = 255;
		Items.Add(Col);
	}

	/*auto Index = 0;
	while (Index < Output.Len())
	{
		FColor Color;
		auto EndIndex = Index;

		EndIndex = Output.Find(",");
		Color.R = FString::FormatAsNumber(Output.)
	}*/

}

TSharedRef<ITableRow> SGelPaletteWidget::GenerateTile(TSharedPtr<FColor> Item, const TSharedRef<STableViewBase>& Table)
{
	auto HSV = Item->ReinterpretAsLinear().LinearRGBToHSV();

	auto Hue = StaticCast<float>(HSV.R);
	auto Saturation = StaticCast<float>(HSV.G);
	auto Value = StaticCast<float>(HSV.B);

	auto Row = SNew(STableRow<TSharedPtr<FColor>>, Table)
		[
			SNew(SBox)
			.Padding(4.0f)
			.HeightOverride(40.0f)
			.WidthOverride(25.0f)
			[
				SNew(SBorder)
				.BorderImage(WhiteBrush.Get())
				.BorderBackgroundColor(Item->ReinterpretAsLinear())
				.ToolTipText(FText::FromString(FString::Printf(TEXT("H: %.0f S: %.3f V: %.3f"), Hue, Saturation, Value)))
			]
		];

	

	return Row;
	
}

void SGelPaletteWidget::OnSelectionMade(TSharedPtr<FColor> SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectedItem)
	{
		SelectionCallback.ExecuteIfBound(SelectedItem->ReinterpretAsLinear().LinearRGBToHSV());		
	}
}

void SGelPaletteWidget::OnItemDoubleClicked(TSharedPtr<FColor> SelectedItem)
{
	SelectionCallback.ExecuteIfBound(SelectedItem->ReinterpretAsLinear().LinearRGBToHSV());

	if (Window)
	{
		Window->HideWindow();
	}

}

