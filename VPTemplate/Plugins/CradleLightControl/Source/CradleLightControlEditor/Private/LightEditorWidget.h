#pragma once
#pragma once

#include "ItemHandle.h"
#include "Slate.h"
#include "Templates/SharedPointer.h"
#include "LightTreeHierarchy.h"
#include "LightPropertyEditor.h"
#include "LightSpecificPropertyEditor.h"
#include "LightItemHeader.h"


class UToolData;

class SLightEditorWidget: public SCompoundWidget
{
public:

    SLATE_BEGIN_ARGS(SLightEditorWidget) {}

    SLATE_ARGUMENT(TSharedPtr<SDockTab>, ToolTab);

    SLATE_END_ARGS();

    void Construct(const FArguments& Args, UToolData* ToolData, EDataSet InDataSet);

    ~SLightEditorWidget();

    void PreDestroy();
    
    void OnTreeSelectionChanged();
    virtual void UpdateExtraLightDetailBox();

    void ClearSelection();

    TWeakPtr<SLightHierarchyWidget> GetTreeWidget();
    TWeakPtr<SLightPropertyEditor> GetLightPropertyEditor();

    FString OpenFileDialog(FString Title, FString StartingPath);
    FString SaveFileDialog(FString Title, FString StartingPath);
    
    UEditorData* GetEditorData() const;

    TSharedRef<SDockTab> Show();
    void Hide();

    void UpdateSaturationGradient(float NewHueValue);

protected:

    SVerticalBox::FSlot& LightHeader();

    SVerticalBox::FSlot& LightPropertyEditor();

    SHorizontalBox::FSlot& LightSpecificPropertyEditor();

    UPROPERTY()
    UEditorData* EditorData;
	EDataSet DataSet;

    TSharedPtr<SBox> ExtraLightDetailBox;

    TSharedPtr<SDockTab> ToolTab;
    TSharedPtr<SLightHierarchyWidget> LightHierarchyWidget;
    TSharedPtr<SLightPropertyEditor> LightPropertyWidget;
    TSharedPtr<SLightSpecificProperties> LightSpecificPropertiesWidget;
    TSharedPtr<SLightItemHeader> LightHeaderWidget;
    TSharedPtr<FActiveTimerHandle> DataAutoSaveTimer;

    TSharedPtr<SVerticalBox> HierarchyVerticalBox; // Vertical box widget encompassing the selection hierarchy.
    TSharedPtr<SVerticalBox> PropertiesVerticalBox; // Vertical box encompassing the general light properties and any properties specific to the tool. Additional slots will go underneath the default properties.
    TSharedPtr<SHorizontalBox> PropertiesHorizontalBox; // Horizontal box which encompasses the properties of the lights. Additional slots will go to the right of the light specific properties.

    // Virtual Only

};