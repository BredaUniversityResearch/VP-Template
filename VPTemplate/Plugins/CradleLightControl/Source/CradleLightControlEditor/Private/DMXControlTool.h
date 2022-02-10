#pragma once

#include "Slate.h"


#include "LightTreeHierarchy.h"
#include "LightPropertyEditor.h"
#include "LightSpecificPropertyEditor.h"
#include "LightItemHeader.h"
#include "LightEditorWidget.h"

class UDMXLight;
class SDMXControlTool : public SLightEditorWidget
{

public:
    
	void Construct(const FArguments& Args, UToolData* ToolData);

    ~SDMXControlTool();

	virtual void UpdateExtraLightDetailBox() override;

    FString GetDXMConfigAssetPath(class UDMXConfigAsset* Asset) const;

private:


    FReply AddLightButtonCallback();

    TSharedRef<SBox> DMXChannelProperties();

    void OnPortSelected();
    ECheckBoxState UseDMXCheckboxState() const;
    void UseDMXCheckboxStateChanged(ECheckBoxState NewState);
    FString DMXConfigObjectPath() const;
    void OnSetDMXConfigAsset(const FAssetData& AssetData);
    TOptional<int> StartingChannelBoxGetValue() const;
    void StartingChannelBoxValueCommitted(int Value, ETextCommit::Type CommitType);

    TSharedPtr<SDMXPortSelector> DMXPortSelector;
};