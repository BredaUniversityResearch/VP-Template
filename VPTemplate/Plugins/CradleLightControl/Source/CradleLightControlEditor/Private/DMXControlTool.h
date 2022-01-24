#pragma once

#include "Slate.h"


#include "LightTreeHierarchy.h"
#include "LightPropertyEditor.h"
#include "LightSpecificPropertyEditor.h"
#include "LightItemHeader.h"

class UDMXLight;
class SDMXControlTool : public SCompoundWidget
{

public:

    SLATE_BEGIN_ARGS(SDMXControlTool) {}

    SLATE_ARGUMENT(TSharedPtr<SDockTab>, ToolTab);

    SLATE_END_ARGS()

        void Construct(const FArguments& Args);

    ~SDMXControlTool();

    void PreDestroy();

    void OnTreeSelectionChanged();

    void UpdateExtraLightDetailBox();

    void ClearSelection();

    TWeakPtr<SLightTreeHierarchy> GetTreeWidget();
    TWeakPtr<SLightPropertyEditor> GetLightPropertyEditor();

    FString OpenFileDialog(FString Title, FString StartingPath);
    FString SaveFileDialog(FString Title, FString StartingPath);

    UToolData* GetToolData() const;

    TSharedRef<SDockTab> Show();
    void Hide();

private:

    void LoadResources();

    SVerticalBox::FSlot& LightHeader();

    FReply AddLightButtonCallback();

    SVerticalBox::FSlot& LightPropertyEditor();

    TSharedRef<SBox> GroupControls();
    TSharedRef<SWidget> GroupControlDropDownLabel(UItemHandle* Item);
    void GroupControlDropDownSelection(UItemHandle* Item, ESelectInfo::Type SelectInfoType);
    FText GroupControlDropDownDefaultLabel() const;
    FText GroupControlLightList() const;

    SHorizontalBox::FSlot& LightSpecificPropertyEditor();

    TSharedRef<SBox> DMXChannelProperties();

    void OnPortSelected();
    ECheckBoxState UseDMXCheckboxState() const;
    void UseDMXCheckboxStateChanged(ECheckBoxState NewState);
    FString DMXConfigObjectPath() const;
    void OnSetDMXConfigAsset(const FAssetData& AssetData);
    TOptional<int> StartingChannelBoxGetValue() const;
    void StartingChannelBoxValueCommitted(int Value, ETextCommit::Type CommitType);


    UToolData* ToolData;

    TSharedPtr<SBox> ExtraLightDetailBox;

    TSharedPtr<SDockTab> ToolTab;
    TSharedPtr<SLightTreeHierarchy> TreeWidget;
    TSharedPtr<SLightPropertyEditor> LightPropertyWidget;
    TSharedPtr<SLightSpecificProperties> LightSpecificWidget;
    TSharedPtr<SLightItemHeader> ItemHeader;
    TSharedPtr<FActiveTimerHandle> DataAutoSaveTimer;

    TSharedPtr<SDMXPortSelector> DMXPortSelector;

    FDelegateHandle ActorSpawnedListenerHandle;


};