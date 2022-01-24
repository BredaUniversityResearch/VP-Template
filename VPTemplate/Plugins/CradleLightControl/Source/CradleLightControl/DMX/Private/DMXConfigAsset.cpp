#pragma once



#include "../Public/DMXConfigAsset.h"

#include "DMXLight.h"
#include "AssetRegistry/AssetRegistryModule.h"

float FDMXLinearChannel::NormalizedToValue(float Normalized)
{
    return Normalized * GetValueRange() + MinValue;
}

uint8 FDMXLinearChannel::NormalizedToDMX(float Normalized)
{
    return StaticCast<uint8>(Normalized * GetDMXValueRange() + MinDMXValue);
}

float FDMXLinearChannel::NormalizeValue(float Value)
{
    return (Value - MinValue) / GetValueRange();
}

uint8 FDMXLinearChannel::ValueToDMX(float Value)
{
    return NormalizedToDMX(NormalizeValue(Value));
}

void FDMXLinearChannel::SetChannel(TMap<int32, uint8>& Channels, void* ValuePtr, int32 StartingChannel)
{
	if (bEnabled)
	{
        auto Value = *reinterpret_cast<float*>(ValuePtr);
        // We subtract 1 to make it more intuitive - first channel in config means starting channel for the light
        Channels.FindOrAdd(StartingChannel + Channel - 1) = ValueToDMX(Value);
		
	}
}

float FDMXLinearChannel::GetValueRange() const
{
    return MaxValue - MinValue;
}

uint8 FDMXLinearChannel::GetDMXValueRange() const
{
    return MaxDMXValue - MinDMXValue;
}

uint8 FDMXToggleChannel::GetDMXValueForState(bool bState)
{
    return bState ? DMXOnValue : DMXOffValue;
}

void FDMXToggleChannel::SetChannel(TMap<int32, uint8>& Channels, void* ValuePtr, int32 StartingChannel)
{
	if (ApplyWhen != EDMXToggleChannelApplication::Never)
	{
        auto bState = *reinterpret_cast<bool*>(ValuePtr);
        auto DMXValue = GetDMXValueForState(bState);

        if (ApplyWhen == EDMXToggleChannelApplication::Always ||
            (bState && ApplyWhen == EDMXToggleChannelApplication::OnOnly) ||
            (!bState && ApplyWhen == EDMXToggleChannelApplication::OffOnly))
        {
            Channels.FindOrAdd(Channel + StartingChannel - 1) = DMXValue;
        }
	}
}

FDMXConfigAssetAction::FDMXConfigAssetAction()
{
    // Register asset types
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    // register AI category so that AI assets can register to it
    AssetCategoryBit = AssetTools.FindAdvancedAssetCategory(FName("DMX"));
    //AssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("DMX")), FText::FromString("DMX"));
}

FText FDMXConfigAssetAction::GetName() const
{
    return FText::FromString("DMX Light Config");
}

FColor FDMXConfigAssetAction::GetTypeColor() const
{
    return FColor::Cyan;
}

uint32 FDMXConfigAssetAction::GetCategories()
{
    return AssetCategoryBit;
}

UClass* FDMXConfigAssetAction::GetSupportedClass() const
{
    return UDMXConfigAsset::StaticClass();
}

void FDMXConfigAssetAction::OpenAssetEditor(const TArray<UObject*>& InObjects,
    TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);


}

UDMXConfigAsset::UDMXConfigAsset()
{
    // Default the max value for the horizontal and vertical channels to 360 instead of 1
    // since maximum of 1 degrees does not make much sense for rotations.
    HorizontalChannel.MaxValue = 360.0f;
    VerticalChannel.MaxValue = 360.0f;

    return;
    // Code to collect all fields which are representative of a DMX channel and add them to an array
    // for them to be applied in a specific order. Issue is finding a way get the data to the channels we are building the DMX channel map.
    // As such, just keeping it here for now as it is not a very important feature, but it would take a while to implement well.
    for (TFieldIterator<FStructProperty> It(GetClass(), EFieldIteratorFlags::ExcludeSuper); It; ++It)
    {
        auto Prop = *It;
        if (Prop->HasMetaData("DMXChannel"))
        {
            FDMXChannel* Channel;

            auto FieldClass = Prop->GetClass();

            Channel = Prop->ContainerPtrToValuePtr<FDMXChannel>(this);
            //auto ScriptStruct = Prop->Struct;
            //ScriptStruct.value
            UE_LOG(LogTemp, Warning, TEXT("%d"), Channel->Channel/*->GetFName().ToString()*/);
            SortedChannels.Add(Channel);
        }
    }
    SortedChannels.Sort([](FDMXChannel& Left, FDMXChannel& Right)
        {
            return Left.ApplicationOrder > Right.ApplicationOrder;
        });
}

void UDMXConfigAsset::SetChannels(UDMXLight* DMXLight, TMap<int32, uint8>& Channels)
{
    auto RGB = DMXLight->GetRGBColor();

    for (auto Constant : ConstantChannels)
    {
        Channels.FindOrAdd(Constant.Channel) = Constant.Value;
    }

    HorizontalChannel.SetChannel(Channels, &DMXLight->Horizontal, DMXLight->StartingChannel);
    VerticalChannel.SetChannel(Channels, &DMXLight->Vertical, DMXLight->StartingChannel);

    IntensityChannel.SetChannel(Channels, &DMXLight->Intensity, DMXLight->StartingChannel);

    RedChannel.SetChannel(Channels, &RGB.R, DMXLight->StartingChannel);
    GreenChannel.SetChannel(Channels, &RGB.G, DMXLight->StartingChannel);
    BlueChannel.SetChannel(Channels, &RGB.B, DMXLight->StartingChannel);

    OnOffChannel.SetChannel(Channels, &DMXLight->bIsEnabled, DMXLight->StartingChannel);

}

void UDMXConfigAsset::SetupChannels(UDMXLight* DMXLight)
{
    /*OnOffChannel.DataPtr = &DMXLight->bIsEnabled;
    HorizontalChannel = */
}

FString UDMXConfigAsset::GetAssetPath()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetData;
    auto Res = AssetRegistryModule.Get().GetAssetsByClass(GetClass()->GetFName(), AssetData);
    check(Res)
    auto TargetAsset = AssetData.FindByPredicate([this](const FAssetData& Asset)
        {
            
            return Asset.AssetName == AssetName;
        });
    if (TargetAsset)
    {
		return TargetAsset->ObjectPath.ToString();	    
    }
    return "";
}
