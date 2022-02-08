#pragma once

#include "CoreMinimal.h"


#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "UObject/NoExportTypes.h"

#include "DMXConfigAsset.generated.h"

USTRUCT(BlueprintType)
struct FDMXChannel
{

    GENERATED_BODY()

    FDMXChannel()
	    : bEnabled(true)
        , Channel(1)
        , ApplicationOrder(0) {}

    virtual void SetChannel(TMap<int32, uint8>& Channels, void* ValuePtr, int32 StartingChannel) {}

    // Should the channel be used?
    UPROPERTY(EditAnywhere)
        bool bEnabled;

    // The channel index for the fixture. NOT 0-based.
    // Channel 1 will correlate to channel 1 on the DMX protocol.
    UPROPERTY(EditAnywhere)
        int32 Channel;

    // Defines when the channel will be applied relative to other channels.
    // Channels with lower value are applied first. This is to be used for cases where a single channel is used for multiple variables,
    // i.e. Intensity and On/Off state sharing a DMX channel.
    //UPROPERTY(EditAnywhere)
        int32 ApplicationOrder;

    //void* DataPtr;
    //uint32 StartingChannel;
};

// Struct representing a DMX channel whose value scales linearly based on
// a range of 8-bit values and a range of float values.
USTRUCT(BlueprintType)
struct CRADLELIGHTCONTROL_API FDMXLinearChannel
    : public FDMXChannel
{
    GENERATED_BODY()

    FDMXLinearChannel()
	    : FDMXChannel()
		, MinDMXValue(0)
		, MaxDMXValue(255)
		, MinValue(0.0f)
		, MaxValue(1.0f)
		
		//, bEnabled(true)
		//, Channel(1)
		{}


    //UFUNCTION(BlueprintCallable)
    // Returns the real value based on a normalized (0.0f - 1.0f) value
    float NormalizedToValue(float Normalized);

    //UFUNCTION(BlueprintCallable)
    // Returns the DMX value based on a normalized (0.0f - 1.0f) value
    uint8 NormalizedToDMX(float Normalized);

    //UFUNCTION(BlueprintCallable)
    // Returns a normalized (0.0f - 1.0f) value based on a real value.
    float NormalizeValue(float Value);

    // Converts a real value into a DMX value
    uint8 ValueToDMX(float Value);

    void SetChannel(TMap<int32, uint8>& Channels, void* ValuePtr, int32 StartingChannel) override;

    // Return the expected range of the real value
    float GetValueRange() const;
    // Return the range of the DMX values on this channel
    uint8 GetDMXValueRange() const;
    
    UPROPERTY(EditAnywhere)
        uint8 MinDMXValue;
    UPROPERTY(EditAnywhere)
        uint8 MaxDMXValue;

    UPROPERTY(EditAnywhere)
        float MinValue;
    UPROPERTY(EditAnywhere)
        float MaxValue;

};

UENUM()
enum EDMXToggleChannelApplication
{
	Always = 0,
    OnOnly,
    OffOnly,
    Never
};

// Struct representing a two-state channel.
// Allows for the user to select in what cases it will apply to avoid overriding other channels in certain cases
USTRUCT(BlueprintType)
struct CRADLELIGHTCONTROL_API FDMXToggleChannel
    : public FDMXChannel
{
    GENERATED_BODY()

        FDMXToggleChannel()
        : DMXOnValue(0)
        , DMXOffValue(255)
        {}




    uint8 GetDMXValueForState(bool bState);

    void SetChannel(TMap<int32, uint8>& Channels, void* ValuePtr, int32 StartingChannel) override;

    uint8 GetDMXValueRange() const;

    UPROPERTY(EditAnywhere)
        uint8 DMXOnValue; // 8-bit value for when the boolean behind the channel is true
    UPROPERTY(EditAnywhere)
        uint8 DMXOffValue; // 8-bit value for when the boolean behind the channel is false

    // Defines under what condition the DMX channel must be updated
    UPROPERTY(EditAnywhere)
        TEnumAsByte<EDMXToggleChannelApplication> ApplyWhen;

    
};

// Struct representing a constant DMX channel. That is to say, a channel which will remain constant at all times, for whatever reason.
USTRUCT(BlueprintType)
struct CRADLELIGHTCONTROL_API FConstDMXChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
        int32 Channel;

    UPROPERTY(EditAnywhere)
        uint8 Value;
};

// Asset type representing a DMX channel configuration.
// This is meant to alleviate the issue of different DMX light fixtures making different use of the values they receive through the protocol

UCLASS(BlueprintType)
class CRADLELIGHTCONTROL_API UDMXConfigAsset : public UObject
{
    GENERATED_BODY()

public:

    UDMXConfigAsset();
    // Get the Asset Path by which the asset can be identified in the asset registry
    FString GetAssetPath();
    void SetChannels(class UDMXLight* DMXLight, TMap<int32, uint8>& Channels);
    void SetupChannels(UDMXLight* DMXLight);
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXToggleChannel OnOffChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXLinearChannel HorizontalChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXLinearChannel VerticalChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXLinearChannel RedChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXLinearChannel GreenChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXLinearChannel BlueChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DMXChannel))
        FDMXLinearChannel IntensityChannel;

    // Channels which are going to have a constant value.
    // If these overlap with non-constant channels, the non-constant channels will be used instead
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FConstDMXChannel> ConstantChannels;

    //UPROPERTY(NonTransactional)
        TArray<FDMXChannel*> SortedChannels;

	UPROPERTY(NonTransactional)
		FName AssetName;
};