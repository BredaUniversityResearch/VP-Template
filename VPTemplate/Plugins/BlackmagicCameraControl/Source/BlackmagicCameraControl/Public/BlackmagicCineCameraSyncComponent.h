#pragma once
#include "BMCCForegroundThreadCallbackHandler.h"
#include "BMCCLens.h"
#include "BMCCVideo.h"

#include "BlackmagicCineCameraSyncComponent.generated.h"

class UCineCameraComponent;
UCLASS(ClassGroup = (VirtualProduction), meta = (BlueprintSpawnableComponent))
class UBlackmagicCineCameraSyncComponent
	: public UActorComponent
	, public FBMCCForegroundThreadCallbackHandler
{
	GENERATED_BODY()
public:
	explicit UBlackmagicCineCameraSyncComponent(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void Activate(bool bReset) override;
	virtual void Deactivate() override;

	virtual void OnLensFocus(BMCCDeviceHandle SourceDevice, const FBMCCLens_Focus& Data) override;
	virtual void OnVideoVideoMode(BMCCDeviceHandle SourceDevice, const FBMCCVideo_VideoMode& VideoMode) override;
	virtual void OnVideoRecordingFormat(BMCCDeviceHandle Source, const FBMCCVideo_RecordingFormat& RecordingFormat) override;
	virtual void OnVendorSpecificCanonLens(BMCCDeviceHandle Source, const FBMCCVendorSpecific_CanonLens& Data) override;
private:
	UCineCameraComponent* Target{ nullptr };
};