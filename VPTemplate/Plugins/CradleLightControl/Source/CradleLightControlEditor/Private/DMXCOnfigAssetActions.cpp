#include "DMXConfigAsset.h"
#include "DMXConfigAssetActions.h"

FDMXConfigAssetAction::FDMXConfigAssetAction()
{
    // Register asset types
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    // register AI category so that AI assets can register to it
    AssetCategoryBit = AssetTools.FindAdvancedAssetCategory(FName("DMX"));
    //AssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("DMX")), FText::FromString("DMX"));
}

FText FDMXConfigAssetAction::GetName() const
{
    return FText::FromString("DMX Light Config");
}

FColor FDMXConfigAssetAction::GetTypeColor() const
{
    return FColor::Cyan;
}

uint32 FDMXConfigAssetAction::GetCategories()
{
    return AssetCategoryBit;
}

UClass* FDMXConfigAssetAction::GetSupportedClass() const
{
    return UDMXConfigAsset::StaticClass();
}

void FDMXConfigAssetAction::OpenAssetEditor(const TArray<UObject*>& InObjects,
    TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);


}