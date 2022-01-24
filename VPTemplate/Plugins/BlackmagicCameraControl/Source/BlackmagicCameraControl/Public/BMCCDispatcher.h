#pragma once
#include "BMCCCallbackHandler.h"

#include "BMCCLens.h"
#include "BMCCVideo.h"
#include "BMCCBattery_Info.h"
#include "BMCCForegroundThreadCallbackHandler.h"
#include "BMCCMedia_TransportMode.h"

#include "BMCCDispatcher.generated.h"

struct BMCCCommandHeader;
class FBMCCCommandMeta;

#define BMCC_CREATE_DISPATCHER(HandlerName, InterfaceFunctionName, PacketDataType) \
UPROPERTY(BlueprintAssignable) F##HandlerName HandlerName; \
virtual void InterfaceFunctionName(BMCCDeviceHandle Source, const PacketDataType& Payload) override { HandlerName.Broadcast(Source, Payload); }

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_Focus, int32, SourceDevice, const FBMCCLens_Focus&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_ApertureFStop, int32, SourceDevice, const FBMCCLens_ApertureFStop&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_ApertureNormalized, int32, SourceDevice, const FBMCCLens_ApertureNormalized&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_ApertureOrdinal, int32, SourceDevice, const FBMCCLens_ApertureOrdinal&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_OpticalImageStabilization, int32, SourceDevice, const FBMCCLens_OpticalImageStabilization&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_SetAbsoluteZoomMm, int32, SourceDevice, const FBMCCLens_SetAbsoluteZoomMm&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_SetAbsoluteZoomNormalized, int32, SourceDevice, const FBMCCLens_SetAbsoluteZoomNormalized&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_SetContinuousZoom, int32, SourceDevice, const FBMCCLens_SetContinuousZoom&, Payload);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMediaTransportModeReceived, int32, SourceDevice, const FBMCCMedia_TransportMode&, Payload);

UCLASS(BlueprintType)
class BLACKMAGICCAMERACONTROL_API UBMCCDispatcher
	: public UObject
	, public FTickableGameObject
	, public FBMCCForegroundThreadCallbackHandler
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(BMCCDispatcherStat, STATGROUP_Tickables); }

	virtual bool IsTickableInEditor() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }

	virtual void OnLensFocus(BMCCDeviceHandle Source, const FBMCCLens_Focus& Focus) override;
	virtual void OnLensApertureFStop(BMCCDeviceHandle Source, const FBMCCLens_ApertureFStop& Aperture) override;
	virtual void OnLensApertureOrdinal(BMCCDeviceHandle Source, const FBMCCLens_ApertureOrdinal& Aperture) override;
	virtual void OnLensApertureNormalized(BMCCDeviceHandle Source, const FBMCCLens_ApertureNormalized& Aperture) override;
	virtual void OnLensOpticalImageStabilization(BMCCDeviceHandle Source, const FBMCCLens_OpticalImageStabilization& ImageStabilization) override;
	virtual void OnLensAbsoluteZoomMm(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomMm& Zoom) override;
	virtual void OnLensAbsoluteZoomNormalized(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomNormalized& Zoom) override;
	virtual void OnLensContinuousZoom(BMCCDeviceHandle Source, const FBMCCLens_SetContinuousZoom& Zoom) override;

	virtual void OnMediaTransportMode(BMCCDeviceHandle Source, const FBMCCMedia_TransportMode& TransportMode) override;

	UPROPERTY(BlueprintAssignable) FOnLens_Focus LensFocusReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_ApertureFStop LensApertureFStopReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_ApertureNormalized LensApertureNormalizedReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_ApertureOrdinal LensApertureOrdinalReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_OpticalImageStabilization LensOpticalImageStabilizationReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_SetAbsoluteZoomMm LensAbsoluteZoomMmReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_SetAbsoluteZoomNormalized LensAbsoluteZoomNormalizedReceived;
	UPROPERTY(BlueprintAssignable) FOnLens_SetContinuousZoom LensSetContinuousZoomReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVideoVideoMode, int32, SourceDevice, const FBMCCVideo_VideoMode&, Payload);
	BMCC_CREATE_DISPATCHER(VideoVideoMode, OnVideoVideoMode, FBMCCVideo_VideoMode)

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVideoRecordingFormat, int32, SourceDevice, const FBMCCVideo_RecordingFormat&, Payload);
	BMCC_CREATE_DISPATCHER(VideoRecordingFormat, OnVideoRecordingFormat, FBMCCVideo_RecordingFormat);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBatteryStatusReceived, int32, SourceDevice, const FBMCCBattery_Info&, Payload);
	BMCC_CREATE_DISPATCHER(BatteryStatusReceived, OnBatteryStatus, FBMCCBattery_Info);

	UPROPERTY(BlueprintAssignable) FOnMediaTransportModeReceived MediaTransportModeReceived;

};
