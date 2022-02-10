#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"


#include "DMXConfigFactory.generated.h"

UCLASS()
class UDMXConfigFactory : public UFactory
{
    GENERATED_BODY()

public:

    UDMXConfigFactory();

    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

    virtual bool ShouldShowInNewMenu() const override { return true ;};

    //virtual uint32 GetMenuCategories() const override;
};