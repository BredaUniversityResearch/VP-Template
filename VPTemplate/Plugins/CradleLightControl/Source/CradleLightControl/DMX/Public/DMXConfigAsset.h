#pragma once

#include "AssetTypeActions_Base.h"

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

    UPROPERTY(EditAnywhere)
        bool bEnabled;

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


USTRUCT(BlueprintType)
struct CRADLELIGHTCONTROL_API FDMXLinearChannel
    : public FDMXChannel
{
    GENERATED_BODY()

    FDMXLinearChannel()
	    : FDMXChannel()
		, MinValue(0.0f)
		, MaxValue(1.0f)
		, MinDMXValue(0)
		, MaxDMXValue(255)
		
		//, bEnabled(true)
		//, Channel(1)
		{}


    //UFUNCTION(BlueprintCallable)
    float NormalizedToValue(float Normalized);

    //UFUNCTION(BlueprintCallable)
    uint8 NormalizedToDMX(float Normalized);

    //UFUNCTION(BlueprintCallable)
    float NormalizeValue(float Value);

    uint8 ValueToDMX(float Value);

    void SetChannel(TMap<int32, uint8>& Channels, void* ValuePtr, int32 StartingChannel) override;

    float GetValueRange() const;
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
        uint8 DMXOnValue;
    UPROPERTY(EditAnywhere)
        uint8 DMXOffValue;

    UPROPERTY(EditAnywhere)
        TEnumAsByte<EDMXToggleChannelApplication> ApplyWhen;

    
};

USTRUCT(BlueprintType)
struct CRADLELIGHTCONTROL_API FConstDMXChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
        int32 Channel;

    UPROPERTY(EditAnywhere)
        uint8 Value;
};

class FDMXConfigAssetAction : public FAssetTypeActions_Base
{
public:
    FDMXConfigAssetAction();
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual uint32 GetCategories() override;
    virtual UClass* GetSupportedClass() const override;
    virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

    EAssetTypeCategories::Type AssetCategoryBit;
};

UCLASS(BlueprintType)
class CRADLELIGHTCONTROL_API UDMXConfigAsset : public UObject
{
    GENERATED_BODY()

public:

    UDMXConfigAsset();
    FString GetAssetPath();
    void SetChannels(class UDMXLight* DMXLight, TMap<int32, uint8>& Channels);
    void SetupChannels(UDMXLight* DMXLight);
    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DMXChannel))
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