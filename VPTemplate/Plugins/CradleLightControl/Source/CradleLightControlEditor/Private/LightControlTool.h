#pragma once

#include "ItemHandle.h"
#include "Slate.h"
#include "Templates/SharedPointer.h"
#include "LightTreeHierarchy.h"
#include "LightPropertyEditor.h"
#include "LightSpecificPropertyEditor.h"
#include "LightItemHeader.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"


class UToolData;

class SLightControlTool : public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SLightControlTool) {}

    SLATE_ARGUMENT(TSharedPtr<SDockTab>, ToolTab);

    SLATE_END_ARGS();
    
    void Construct(const FArguments& Args, UToolData* ToolData);

    ~SLightControlTool();

    void PreDestroy();
    
    void ActorSpawnedCallback(AActor* Actor);
    void OnTreeSelectionChanged();
    void UpdateExtraLightDetailBox();
    
    void ClearSelection();

    TWeakPtr<SLightTreeHierarchy> GetTreeWidget();
    TWeakPtr<SLightPropertyEditor> GetLightPropertyEditor();

    FString OpenFileDialog(FString Title, FString StartingPath);
    FString SaveFileDialog(FString Title, FString StartingPath);

    //FSlateBrush& GetIcon(EIconType Icon);

    void UpdateLightList();


    static void UpdateItemData(UItemHandle* ItemHandle);
    void VerifyTreeData();

    UToolData* GetToolData() const;

    TSharedRef<SDockTab> Show();
    void Hide();


private:

    void LoadResources();

    SVerticalBox::FSlot& LightHeader();
     
    void MetaDataSaveExtension(TSharedPtr<FJsonObject> RootJson);
    void MetaDataLoadExtension(TSharedPtr<FJsonObject> RootJson);

    SVerticalBox::FSlot& LightPropertyEditor();

    TSharedRef<SBox> LightTransformViewer();
    FReply SelectItemInScene();
    FReply SelectItemParent();
    bool SelectItemParentButtonEnable() const;
    FText GetItemParentName() const;
    FText GetItemPosition() const;
    FText GetItemRotation() const;
    FText GetItemScale() const;

    TSharedRef<SBox> GroupControls();
    TSharedRef<SWidget> GroupControlDropDownLabel(UItemHandle* Item);
    void GroupControlDropDownSelection(UItemHandle* Item, ESelectInfo::Type SelectInfoType);
    FText GroupControlDropDownDefaultLabel() const;
    FText GroupControlLightList() const;

    SHorizontalBox::FSlot& LightSpecificPropertyEditor();

    UToolData* ToolData;

    TSharedPtr<SBox> ExtraLightDetailBox;

    TSharedPtr<SDockTab> ToolTab;
    TSharedPtr<SLightTreeHierarchy> TreeWidget;
    TSharedPtr<SLightPropertyEditor> LightPropertyWidget;
    TSharedPtr<SLightSpecificProperties> LightSpecificWidget;
    TSharedPtr<SLightItemHeader> ItemHeader;
    TSharedPtr<FActiveTimerHandle> DataAutoSaveTimer;

    FDelegateHandle ActorSpawnedListenerHandle;
    FDelegateHandle OnWorldChangedDelegateHandle;
    FDelegateHandle OnWorldCleanupStartedDelegate;

};