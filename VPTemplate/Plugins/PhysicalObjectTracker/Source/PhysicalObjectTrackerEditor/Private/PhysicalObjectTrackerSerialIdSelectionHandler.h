#pragma once


class FDetectTrackerShakeTask;
class UPhysicalObjectTrackerSerialId;

class FPhysicalObjectTrackerSerialIdSelectionHandler
{
public:
    TSharedRef<FExtender> CreateMenuExtender(const TArray<FAssetData>& SelectedAssets);

private:

    void MenuExtenderImpl(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

    void StartDeviceSelection(UPhysicalObjectTrackerSerialId* SerialIdAsset);
    void StopDeviceSelection();

    TUniquePtr<FDetectTrackerShakeTask> m_ShakeDetectTask;
    TSharedPtr<SNotificationItem> m_ShakeProcessNotification;

};