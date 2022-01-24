#include "BMCCDispatcher.h"

#include "BMCCCommandMeta.h"

UBMCCDispatcher::CachedMessage::CachedMessage(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8>& ArrayView)
	: Source(Source)
	, Header(Header)
	, CommandMetaData(&CommandMetaData)
	, SerializedData(ArrayView)
{
}

void UBMCCDispatcher::Tick(float DeltaTime)
{
	DispatchPendingMessages();
}

void UBMCCDispatcher::OnLensFocus(BMCCDeviceHandle Source, const FBMCCLens_Focus& Focus)
{
	LensFocusReceived.Broadcast(Source, Focus);
}

void UBMCCDispatcher::OnLensApertureFStop(BMCCDeviceHandle Source, const FBMCCLens_ApertureFStop& Aperture)
{
	IBMCCCallbackHandler::OnLensApertureFStop(Source, Aperture);
}

void UBMCCDispatcher::OnLensApertureOrdinal(BMCCDeviceHandle Source, const FBMCCLens_ApertureOrdinal& Aperture)
{
	IBMCCCallbackHandler::OnLensApertureOrdinal(Source, Aperture);
}

void UBMCCDispatcher::OnLensApertureNormalized(BMCCDeviceHandle Source, const FBMCCLens_ApertureNormalized& Aperture)
{
	IBMCCCallbackHandler::OnLensApertureNormalized(Source, Aperture);
}

void UBMCCDispatcher::OnLensOpticalImageStabilization(BMCCDeviceHandle Source, const FBMCCLens_OpticalImageStabilization& ImageStabilization)
{
	IBMCCCallbackHandler::OnLensOpticalImageStabilization(Source, ImageStabilization);
}

void UBMCCDispatcher::OnLensAbsoluteZoomMm(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomMm& Zoom)
{
	IBMCCCallbackHandler::OnLensAbsoluteZoomMm(Source, Zoom);
}

void UBMCCDispatcher::OnLensAbsoluteZoomNormalized(BMCCDeviceHandle Source, const FBMCCLens_SetAbsoluteZoomNormalized& Zoom)
{
	IBMCCCallbackHandler::OnLensAbsoluteZoomNormalized(Source, Zoom);
}

void UBMCCDispatcher::OnLensContinuousZoom(BMCCDeviceHandle Source, const FBMCCLens_SetContinuousZoom& Zoom)
{
	IBMCCCallbackHandler::OnLensContinuousZoom(Source, Zoom);
}

void UBMCCDispatcher::OnMediaTransportMode(BMCCDeviceHandle Source, const FBMCCMedia_TransportMode& TransportMode)
{
	MediaTransportModeReceived.Broadcast(Source, TransportMode);
}
