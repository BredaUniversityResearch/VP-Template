#pragma once
#include "AssetTypeActions_Base.h"


// Asset action object needed to better customize the available actions for our custom DMXConfig asset

class FDMXConfigAssetAction : public FAssetTypeActions_Base
{
public:
    FDMXConfigAssetAction();
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual uint32 GetCategories() override;
    virtual UClass* GetSupportedClass() const override;
    virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

    EAssetTypeCategories::Type AssetCategoryBit;
};
