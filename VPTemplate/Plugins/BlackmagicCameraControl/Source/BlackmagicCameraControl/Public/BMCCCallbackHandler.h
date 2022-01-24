#pragma once
#include "BMCCDeviceHandle.h"
#include "BMCCLens.h"
#include "BMCCVendorSpecific.h"
#include "BMCCVideo.h"

#include "BMCCCallbackHandler.generated.h"

struct FBMCCMedia_TransportMode;
struct FBMCCBattery_Info;
struct BMCCCommandHeader;
class FBMCCCommandMeta;

UINTERFACE(MinimalAPI, Blueprintable)
class UBMCCCallbackHandler : public UInterface
{
	GENERATED_BODY()
};

class IBMCCCallbackHandler
{
	GENERATED_BODY()
public:
	virtual void OnDataReceived(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView) = 0;

	virtual void OnLensFocus(BMCCDeviceHandle Source, const FBMCCLens_Focus& Focus) {}
	virtual void OnLensApertureFStop(BMCCDeviceHandle Source, const FBMCCLens_ApertureFStop& Aperture) {}
	virtual void OnLensApertureNormalized(BMCCDeviceHandle Source, const FBMCCLens_ApertureNormalized& Aperture) {}
	virtual void OnLensApertureOrdinal(BMCCDeviceHandle Source, const FBMCCLens_ApertureOrdinal& Aperture) {}
	virtual void OnLensOpticalImageStabilization(BMCCDeviceHandle Source, const FBMCCLens_OpticalImageStabilization& ImageStabilization) {}
	virtual void OnLensAbsoluteZoomMm(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomMm& Zoom) {}
	virtual void OnLensAbsoluteZoomNormalized(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomNormalized& Zoom) {}
	virtual void OnLensContinuousZoom(BMCCDeviceHandle Source, const FBMCCLens_SetContinuousZoom& Zoom) {}

	virtual void OnVideoVideoMode(BMCCDeviceHandle Source, const FBMCCVideo_VideoMode& Data) {}
	virtual void OnVideoGain(BMCCDeviceHandle Source, const FBMCCVideo_Gain& Data) {}
	virtual void OnVideoRecordingFormat(BMCCDeviceHandle Source, const FBMCCVideo_RecordingFormat& RecordingFormat) {};

	virtual void OnBatteryStatus(BMCCDeviceHandle Source, const FBMCCBattery_Info& BatteryInfo) {};
	virtual void OnMediaTransportMode(BMCCDeviceHandle Source, const FBMCCMedia_TransportMode& TransportMode) {};

	virtual void OnVendorSpecificCanonLens(BMCCDeviceHandle Source, const FBMCCVendorSpecific_CanonLens& Data) {};
};
