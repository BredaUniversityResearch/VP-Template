#include "ObjectTrackingPacketData.h"

FObjectTrackingPacketData::FObjectTrackingPacketData(
    const FString& InTrackerAssetName, 
    const FString& InTrackerSerialId, 
    const FTransform& InTrackerTransform)
        :
IData(EPacketDataType::TRACKERDATA),
TrackerAssetName(InTrackerAssetName),
TrackerSerialId(InTrackerSerialId),
TrackerTransform(InTrackerTransform)
{}

FObjectTrackingData::~FObjectTrackingPacketData()
{}

void FObjectTrackingPacketData::Serialize(TSharedPtr<FJsonObject> JsonDataObject) const
{
    JsonDataObject->SetStringField("TrackerAssetName", TrackerAssetName);
    JsonDataObject->SetStringField("TrackerSerialId", TrackerSerialId);

    //TODO: make sure this doesn't delete when the function/scope ends. SetObjectField might not increase the ref count?
    const TSharedPtr<FJsonObject> jsonTransform = MakeShareable(new FJsonObject());
    jsonTransform->SetStringField("Rotation", TrackerTransform.GetRotation().ToString());
    jsonTransform->SetStringField("Scale", TrackerTransform.GetScale3D().ToString());
    jsonTransform->SetStringField("Translation", TrackerTransform.GetTranslation().ToString());

    JsonDataObject->SetObjectField("TrackerTransform", jsonTransform);
}