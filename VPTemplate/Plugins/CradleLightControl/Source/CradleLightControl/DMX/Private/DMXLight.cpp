#include "DMXLight.h"
#include "IO/DMXPortManager.h"

#include "AssetRegistry/AssetRegistryModule.h"


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

FPlatformTypes::uint8 UDMXLight::LoadFromJson(TSharedPtr<FJsonObject> JsonObject)
{
    auto PortGUID = JsonObject->GetStringField("OutputPortGUID");// , OutputPort.IsValid() ? OutputPort->GetPortGuid().ToString() : "");
    bDMXEnabled = JsonObject->GetBoolField("DMXEnabled");//, bDMXEnabled);
    auto ConfigObjectPath = JsonObject->GetStringField("ConfigObjectPath");// , ObjectPath);
    StartingChannel = JsonObject->GetNumberField("StartingChannel");// , StartingChannel);

    if (!PortGUID.IsEmpty())
    {
        OutputPort = FDMXPortManager::Get().FindOutputPortByGuid(FGuid(PortGUID));
    }
    else
        OutputPort = nullptr;

    if (!ConfigObjectPath.IsEmpty())
    {
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        auto AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FName(ConfigObjectPath));
        if (ensure(AssetData.IsValid()))
            UE_LOG(LogTemp, Warning, TEXT("Could not load DMX Config asset with object path: %s"), *ConfigObjectPath);

    	Config = Cast<UDMXConfigAsset>(AssetData.GetAsset());
        Config->AssetName = AssetData.AssetName;
    }
    else
        Config = nullptr;

    auto Res = Super::LoadFromJson(JsonObject);


    return Res;
}

TSharedPtr<FJsonObject> UDMXLight::SaveAsJson()
{


    Vertical = Config->VerticalChannel.NormalizeValue(Vertical);
    Horizontal = Config->HorizontalChannel.NormalizeValue(Horizontal);

    auto JsonObject = Super::SaveAsJson();

    Vertical = Config->VerticalChannel.NormalizedToValue(Vertical);
    Horizontal = Config->HorizontalChannel.NormalizedToValue(Horizontal);

    FString ObjectPath = "";

    if (Config)
    {
        /*FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        TArray<FAssetData> AssetData;
        AssetRegistryModule.Get().GetAssetsByClass(Config->GetClass()->GetFName(), AssetData);
        ObjectPath = AssetData[0].ObjectPath.ToString();*/
        ObjectPath = Config->GetAssetPath();
    }

    JsonObject->SetStringField("OutputPortGUID", OutputPort.IsValid() ? OutputPort->GetPortGuid().ToString() : "");
    JsonObject->SetBoolField("DMXEnabled", bDMXEnabled);
    JsonObject->SetStringField("ConfigObjectPath", ObjectPath);
    JsonObject->SetNumberField("StartingChannel", StartingChannel);


    return JsonObject;
}

void UDMXLight::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);
	if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
	{
        UpdateDMXChannels();
	}
}

void UDMXLight::UpdateDMXChannels()
{
    if (OutputPort && bDMXEnabled && Config)
    {
        TMap<int32, uint8> DMXChannels;

        Config->SetChannels(this, DMXChannels);

        OutputPort->SendDMX(1, DMXChannels);
    }
}
