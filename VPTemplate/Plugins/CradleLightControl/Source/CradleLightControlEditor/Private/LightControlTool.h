#pragma once

#include "ItemHandle.h"
#include "Slate.h"
#include "Templates/SharedPointer.h"
#include "LightTreeHierarchy.h"
#include "LightPropertyEditor.h"
#include "LightSpecificPropertyEditor.h"
#include "LightItemHeader.h"

#include "LightEditorWidget.h"

class UToolData;

class SLightControlTool : public SLightEditorWidget
{
public:
    
    void Construct(const FArguments& Args, UToolData* ToolData);

    ~SLightControlTool();

    void PreDestroy();
    void ActorSpawnedCallback(AActor* Actor);

    virtual void UpdateExtraLightDetailBox() override;

    void UpdateLightList();
    
    static void UpdateItemDasta(UBaseLight* BaseLight);
    void VerifyTreseData();

private:

    void MetaDataSaveExtension(TSharedPtr<FJsonObject> RootJson);
    void MetaDataLoadExtension(TSharedPtr<FJsonObject> RootJson);

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

    FDelegateHandle ActorSpawnedListenerHandle;
    FDelegateHandle OnWorldChangedDelegateHandle;
    FDelegateHandle OnWorldCleanupStartedDelegate;

};