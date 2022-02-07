#include "DMXConfigFactory.h"

#include "DMXConfigAsset.h"
#include "AssetTypeCategories.h"

UDMXConfigFactory::UDMXConfigFactory()
{
    SupportedClass = UDMXConfigAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* UDMXConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    auto Asset = NewObject<UDMXConfigAsset>(InParent, InClass, InName, Flags);
    
    return Asset;
}
