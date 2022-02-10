#include "DMXLight.h"
#include "IO/DMXPortManager.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "CradleLightControl.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"

float UDMXLight::GetHorizontalNormalized() const
{
    if (Config)
        return Config->HorizontalChannel.NormalizeValue(Horizontal);
    return 0.0f;
}

float UDMXLight::GetVerticalNormalized() const
{
    if (Config)
        return Config->VerticalChannel.NormalizeValue(Vertical);
    return 0.0f;
}

void UDMXLight::SetEnabled(bool bNewState)
{
    Super::SetEnabled(bNewState);
    UpdateDMXChannels();
}

void UDMXLight::SetLightIntensity(float NormalizedValue)
{
    Super::SetLightIntensity(NormalizedValue);
    UpdateDMXChannels();
}

void UDMXLight::SetHue(float NewValue)
{
    Super::SetHue(NewValue);
    UpdateDMXChannels();
}

void UDMXLight::SetSaturation(float NewValue)
{
    Super::SetSaturation(NewValue);
    UpdateDMXChannels();
}

void UDMXLight::AddHorizontal(float NormalizedDegrees)
{

    if (Config)
        Horizontal += NormalizedDegrees * Config->HorizontalChannel.GetValueRange();
    UpdateDMXChannels();
}

void UDMXLight::AddVertical(float NormalizedDegrees)
{
    if (Config)
        Vertical += NormalizedDegrees * Config->VerticalChannel.GetValueRange();
    UpdateDMXChannels();
}

::ELightControlLoadingResult UDMXLight::LoadFromJson(TSharedPtr<FJsonObject> JsonObject)
{
    auto PortGUID = JsonObject->GetStringField("OutputPortGUID");
    bDMXEnabled = JsonObject->GetBoolField("DMXEnabled");
    auto ConfigObjectPath = JsonObject->GetStringField("ConfigObjectPath");
    StartingChannel = JsonObject->GetNumberField("StartingChannel");

    // The output port is identified by a GUID in the JSON file, so we find the port object by that GUID if it was specified
    if (!PortGUID.IsEmpty())
    {
        OutputPort = FDMXPortManager::Get().FindOutputPortByGuid(FGuid(PortGUID));
    }
    else
        OutputPort = nullptr;

    // The config  reference is saved in JSON via an asset path
    // If one was given, we need to find that asset by the path
    if (!ConfigObjectPath.IsEmpty())
    {
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        auto AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(ConfigObjectPath));
        if (ensure(AssetData.IsValid()))
            UE_LOG(LogCradleLightControl, Warning, TEXT("Could not load DMX Config asset with object path: %s"), *ConfigObjectPath);

        Config = Cast<UDMXConfigAsset>(AssetData.GetAsset());
        Config->AssetName = AssetData.AssetName;
    }
    else
        Config = nullptr;

    // Load the base light properties
    auto Res = Super::LoadFromJson(JsonObject);


    return Res;
}

TSharedPtr<FJsonObject> UDMXLight::SaveAsJson()
{
    if (Config)
    {
        Vertical = Config->VerticalChannel.NormalizeValue(Vertical);
        Horizontal = Config->HorizontalChannel.NormalizeValue(Horizontal);
    }

    auto JsonObject = Super::SaveAsJson();

    if (Config)
    {
        Vertical = Config->VerticalChannel.NormalizedToValue(Vertical);
        Horizontal = Config->HorizontalChannel.NormalizedToValue(Horizontal);
    }

    FString ObjectPath = "";

    if (Config)
    {
        ObjectPath = Config->GetAssetPath();
    }

    // We leave the port GUID empty if the output port is not specified
    JsonObject->SetStringField("OutputPortGUID", OutputPort.IsValid() ? OutputPort->GetPortGuid().ToString() : "");
    JsonObject->SetBoolField("DMXEnabled", bDMXEnabled);
    JsonObject->SetStringField("ConfigObjectPath", ObjectPath);
    JsonObject->SetNumberField("StartingChannel", StartingChannel);


    return JsonObject;
}

#if WITH_EDITOR
void UDMXLight::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
    // When this object is transacted, we need to send new DMX signals reflecting that. Hence this.
    Super::PostTransacted(TransactionEvent);
    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
    {
        UpdateDMXChannels();
    }
}
#endif
void UDMXLight::UpdateDMXChannels()
{
    // We need a specified OutputPort and Config to be able to send valid DMX signals
    if (OutputPort && bDMXEnabled && Config)
    {
        TMap<int32, uint8> DMXChannels;

        Config->SetChannels(this, DMXChannels);

        // SendDMX() will only apply to the channels which are given values in the Map,
        // so it will not overwrite the channels used by other lights unless they are made to overlap
        OutputPort->SendDMX(1, DMXChannels);
    }
}