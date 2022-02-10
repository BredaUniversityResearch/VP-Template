#pragma once

#include "CoreMinimal.h"
#include "LightControlLoadingResult.h"

#include "EditorData.generated.h"

class UBaseLight;
class UItemHandle;
class UToolData;

// Delegates to enable communication between the tool data and the editor module, as well as achieve better responsiveness to editor transactions
DECLARE_DELEGATE(FClearSelectionDelegate);
DECLARE_DELEGATE_RetVal_TwoParams(FString, FLightJsonFileDialogDelegate, FString /*Title*/, FString /*StartDir*/);
DECLARE_DELEGATE(FOnTreeStructureChangedDelegate);
DECLARE_DELEGATE_TwoParams(FItemExpansionChangedDelegate, UItemHandle* /*ItemHandle*/, bool /*bContinueRecursively*/);
DECLARE_DELEGATE_OneParam(FOnMasterLightTransactedDelegate, UItemHandle& /*MasterLightHandle*/); // Pass by reference to ensure it's not nullptr 
DECLARE_DELEGATE_OneParam(FOnToolDataLoadedDelegate, ELightControlLoadingResult /*LoadingResult*/);

DECLARE_DELEGATE_OneParam(FMetaDataExtension, TSharedPtr<FJsonObject> /*RootJsonObject*/)

// Top UObject that is responsible for the hierarchy of Item Handles and lights
// Needed in order for the editor's transaction system to affect drag-drop operations

UCLASS(BlueprintType)
class UEditorData : public UObject
{
    GENERATED_BODY()
public:

    UEditorData();

    ~UEditorData();
    
    UItemHandle* AddItem();

    void SetToolData(UToolData* ToolData);
    void SetWidgetRef(class SLightEditorWidget& Widget);

    UToolData* GetToolData() const { return ToolData; }

    void OnToolDataTransacted(const FTransactionObjectEvent& TransactionEvent);

    void PostLightTransacted(const FTransactionObjectEvent& TransactionEvent, UBaseLight& Light);

    void ClearAllData();

    bool IsAMasterLightSelected(); // Returns true if any light or group containing lights is selected
    bool IsSingleGroupSelected(); // Returns true if a single group is selected. False if any light items are selected, or no group is selected.
    bool MultipleItemsSelected(); // Returns true if multiple items are selected. False if a group with multiple items is selected.
    bool MultipleLightsInSelection(); // Returns true if the selected items total will affect more than one light item.
    UItemHandle* GetMasterLight(); // Returns the currently selected master light
    UItemHandle* GetSelectedGroup(); // Returns the first selected item if it is a group.
    UItemHandle* GetSingleSelectedItem(); // Returns GetSelectedGroup() if possible, GetMasterLight() otherwise.
    const TArray<UItemHandle*>& GetSelectedLights(); // Returns all selected lights and lights under selected groups.
    TArray<UItemHandle*> GetSelectedItems(); // Returns all selected items, irrelevant of their type

    void BeginTransaction();

    FReply SaveCallBack(); // Called when a save is requested by the user. Will act as "Save As" if no preset path is known.
    FReply SaveAsCallback(); // Called when a "save as" is requested by the user. Will invoke a file dialogue window.
    void SaveStateToJson(FString Path, bool bUpdatePresetPath = true); // Saves the state of the hierarchy to a JSON file.
    FReply LoadCallBack(); // Opens a file dialogue window for the user to select a new preset.
    void LoadStateFromJSON(FString Path, bool bUpdatePresetPath = true); // Loads the state of the hierarchy from a JSON file.

    void AutoSave(); // Called periodically or when certain events happen in the editor, like it shutting down.

    TSharedPtr<FJsonObject> OpenMetaDataJson(); // Returns the MetaData JSON object.

    void SaveMetaData();
    void LoadMetaData();


    bool bCurrentlyLoading;
    FString ToolPresetPath; // The last used path when saving or loading a preset. If empty, will resort to an autosave file.

    FMetaDataExtension MetaDataSaveExtension; // Possible extension to how the metadata is being saved.
    FMetaDataExtension MetaDataLoadExtension; // Possible extension to how the metadata is being loaded.

    FClearSelectionDelegate ClearSelectionDelegate;
    FLightJsonFileDialogDelegate SaveFileDialog;
    FLightJsonFileDialogDelegate OpenFileDialog;

    FOnTreeStructureChangedDelegate TreeStructureChangedDelegate; // Callback for whenever the structure of the tree hierarchy changes. Updates the relevant slate widget.
    FItemExpansionChangedDelegate ItemExpansionChangedDelegate; // Callback for when an item's expansion is changed.
    FOnToolDataLoadedDelegate OnToolDataLoaded; // Callback when the tool data is loaded. Used to generate the necessary widgets for the items.

    FTimerHandle AutoSaveTimer;

    // List of all the root items in the dataset. Each root item may or may not have children.
    UPROPERTY()
        TArray<UItemHandle*> RootItems;

    // 1D list of all items in the tree, no matter if they are parented or not
    UPROPERTY()
        TArray<UItemHandle*> ListOfTreeItems;
    
    // 1D list of all handles which hold lights items as opposed to being groups.
    UPROPERTY()
        TArray<UItemHandle*> ListOfLightItems;


    // List of all items selected by the user via the UI
    UPROPERTY()
        TArray<UItemHandle*> SelectedItems;

    // List of all lights under selected items.
    UPROPERTY()
        TArray<UItemHandle*> LightsUnderSelection;
    // The current master light which determines the values shown in the UI
    UPROPERTY()
        UItemHandle* SelectionMasterLight;

private:
    UToolData* ToolData;

    SLightEditorWidget* OwningWidget;
};
