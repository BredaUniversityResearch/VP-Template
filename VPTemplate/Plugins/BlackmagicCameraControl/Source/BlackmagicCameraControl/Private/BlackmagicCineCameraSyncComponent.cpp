#include "BlackmagicCineCameraSyncComponent.h"

#include "BlackmagicCameraControl.h"
#include "CineCameraComponent.h"

UBlackmagicCineCameraSyncComponent::UBlackmagicCineCameraSyncComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
}

void UBlackmagicCineCameraSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DispatchPendingMessages();
}

void UBlackmagicCineCameraSyncComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	UCineCameraComponent* component = GetOwner()->FindComponentByClass<UCineCameraComponent>();
	Target = component;
	if (component == nullptr)
	{
		UE_LOG(LogBlackmagicCameraControl, Error, TEXT("Could not find CineCameraComponent on attached actor"));
	}

	FBMCCService& service = IModularFeatures::Get().GetModularFeature<FBMCCService>(FBMCCService::GetModularFeatureName());
	service.SubscribeMessageReceivedHandler(this);
}

void UBlackmagicCineCameraSyncComponent::Deactivate()
{
	Super::Deactivate();

	FBMCCService& service = IModularFeatures::Get().GetModularFeature<FBMCCService>(FBMCCService::GetModularFeatureName());
	service.UnsubscribeMessageReceivedHandler(this);
}

void UBlackmagicCineCameraSyncComponent::OnLensFocus(int32 SourceDevice, const FBMCCLens_Focus& Data)
{

}

void UBlackmagicCineCameraSyncComponent::OnVideoVideoMode(int32 SourceDevice, const FBMCCVideo_VideoMode& VideoMode)
{
	int a = 6;
}

void UBlackmagicCineCameraSyncComponent::OnVideoRecordingFormat(BMCCDeviceHandle Source, const FBMCCVideo_RecordingFormat& RecordingFormat)
{
	FCameraFilmbackSettings& filmbackSettings = Target->Filmback;
	//filmbackSettings.SensorWidth = RecordingFormat.FrameWidthPixels;
	//filmbackSettings.SensorHeight = RecordingFormat.FrameHeightPixels;
	filmbackSettings.SensorAspectRatio = RecordingFormat.FrameWidthPixels / RecordingFormat.FrameHeightPixels;
}

void UBlackmagicCineCameraSyncComponent::OnVendorSpecificCanonLens(BMCCDeviceHandle Source, const FBMCCVendorSpecific_CanonLens& Data)
{
	UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Received lens info: %s"), *Data.InfoString);
}

