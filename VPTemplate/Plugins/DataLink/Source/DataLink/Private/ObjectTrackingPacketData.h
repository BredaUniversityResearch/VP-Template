#pragma once
#include "DataPacket.h"

class FObjectTrackingPacketData final : public FDataPacket::IData
{

public:

    FObjectTrackingPacketData(const FString& TrackerAssetName, const FString& TrackerSerialId, const FTransform& TrackerTransform);
    virtual ~FObjectTrackingPacketData() override;

    virtual void Serialize(TSharedPtr<FJsonObject> JsonDataObject) const override;

private:

    FString TrackerAssetName;
    FString TrackerSerialId;
    FTransform TrackerTransform;

};