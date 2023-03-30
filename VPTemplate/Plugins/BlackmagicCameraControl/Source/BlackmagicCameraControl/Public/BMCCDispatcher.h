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

UCLASS(BlueprintType)
class BLACKMAGICCAMERACONTROL_API UBMCCDispatcher
	: public UObject
	, public FTickableGameObject
	, public FBMCCForegroundThreadCallbackHandler
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(BMCCDispatcherStat, STATGROUP_Tickables); }

	virtual bool IsTickableInEditor() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_Focus, int32, SourceDevice, const FBMCCLens_Focus&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_Focus OnLens_Focus;
	virtual void OnLensFocus(BMCCDeviceHandle Source, const FBMCCLens_Focus& Payload) override { OnLens_Focus.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_ApertureFStop, int32, SourceDevice, const FBMCCLens_ApertureFStop&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_ApertureFStop OnLens_ApertureFStop;
	virtual void OnLensApertureFStop(BMCCDeviceHandle Source, const FBMCCLens_ApertureFStop& Payload) override { OnLens_ApertureFStop.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_ApertureNormalized, int32, SourceDevice, const FBMCCLens_ApertureNormalized&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_ApertureNormalized OnLens_ApertureNormalized;
	virtual void OnLensApertureNormalized(BMCCDeviceHandle Source, const FBMCCLens_ApertureNormalized& Payload) override { OnLens_ApertureNormalized.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_ApertureOrdinal, int32, SourceDevice, const FBMCCLens_ApertureOrdinal&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_ApertureOrdinal OnLens_ApertureOrdinal;
	virtual void OnLensApertureOrdinal(BMCCDeviceHandle Source, const FBMCCLens_ApertureOrdinal& Payload) override { OnLens_ApertureOrdinal.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_OpticalImageStabilization, int32, SourceDevice, const FBMCCLens_OpticalImageStabilization&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_OpticalImageStabilization OnLens_OpticalImageStabilization;
	virtual void OnLensOpticalImageStabilization(BMCCDeviceHandle Source, const FBMCCLens_OpticalImageStabilization& Payload) override { OnLens_OpticalImageStabilization.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_SetAbsoluteZoomMm, int32, SourceDevice, const FBMCCLens_SetAbsoluteZoomMm&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_SetAbsoluteZoomMm OnLens_SetAbsoluteZoomMm;
	virtual void OnLensAbsoluteZoomMm(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomMm& Payload) override { OnLens_SetAbsoluteZoomMm.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_SetAbsoluteZoomNormalized, int32, SourceDevice, const FBMCCLens_SetAbsoluteZoomNormalized&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_SetAbsoluteZoomNormalized OnLens_SetAbsoluteZoomNormalized; virtual void OnLensAbsoluteZoomNormalized(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomNormalized& Payload) override { OnLens_SetAbsoluteZoomNormalized.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLens_SetContinuousZoom, int32, SourceDevice, const FBMCCLens_SetContinuousZoom&, Payload);
	UPROPERTY(BlueprintAssignable) FOnLens_SetContinuousZoom OnLens_SetContinuousZoom;
	virtual void OnLensContinuousZoom(BMCCDeviceHandle Source, const FBMCCLens_SetContinuousZoom& Payload) override { OnLens_SetContinuousZoom.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMediaTransportModeReceived, int32, SourceDevice, const FBMCCMedia_TransportMode&, Payload);
	UPROPERTY(BlueprintAssignable) FOnMediaTransportModeReceived OnMediaTransportModeReceived;
	virtual void OnMediaTransportMode(BMCCDeviceHandle Source, const FBMCCMedia_TransportMode& Payload) override { OnMediaTransportModeReceived.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVideoVideoMode, int32, SourceDevice, const FBMCCVideo_VideoMode&, Payload);
	UPROPERTY(BlueprintAssignable) FVideoVideoMode VideoVideoMode;
	virtual void OnVideoVideoMode(BMCCDeviceHandle Source, const FBMCCVideo_VideoMode& Payload) override { VideoVideoMode.Broadcast(Source, Payload); }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVideoRecordingFormat, int32, SourceDevice, const FBMCCVideo_RecordingFormat&, Payload);
	UPROPERTY(BlueprintAssignable) FVideoRecordingFormat VideoRecordingFormat;
	virtual void OnVideoRecordingFormat(BMCCDeviceHandle Source, const FBMCCVideo_RecordingFormat& Payload) override { VideoRecordingFormat.Broadcast(Source, Payload); };
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBatteryStatusReceived, int32, SourceDevice, const FBMCCBattery_Info&, Payload);
	UPROPERTY(BlueprintAssignable) FBatteryStatusReceived BatteryStatusReceived;
	virtual void OnBatteryStatus(BMCCDeviceHandle Source, const FBMCCBattery_Info& Payload) override { BatteryStatusReceived.Broadcast(Source, Payload); };

};
