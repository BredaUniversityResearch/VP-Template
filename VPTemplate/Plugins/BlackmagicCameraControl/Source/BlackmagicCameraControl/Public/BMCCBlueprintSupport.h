#pragma once
#include "BMCCFixed16.h"

#include "BMCCBlueprintSupport.generated.h"

class UBMCCDispatcher;
UCLASS(BlueprintType)
class UBMCCBlueprintSupport: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UBMCCDispatcher* GetBlackmagicCameraControlDispatcher();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Fixed16 to Float", CompactNodeTitle = "->", BlueprintAutocast), Category = "Camera|Blackmagic Camera Control")
	static float Conv_Fixed16ToFloat(FBMCCFixed16 Value);
};