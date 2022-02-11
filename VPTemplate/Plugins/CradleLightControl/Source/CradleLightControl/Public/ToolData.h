#pragma once

#include "CoreMinimal.h"

#include "LightControlLoadingResult.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"

#include "ToolData.generated.h"

class UBaseLight;
class UItemHandle;

// Delegates to enable communication between the tool data and the editor module, as well as achieve better responsiveness to editor transactions
DECLARE_DELEGATE(FClearSelectionDelegate);
DECLARE_DELEGATE_RetVal_TwoParams(FString, FLightJsonFileDialogDelegate, FString /*Title*/, FString /*StartDir*/);
DECLARE_DELEGATE(FOnTreeStructureChangedDelegate);
DECLARE_DELEGATE_TwoParams(FItemExpansionChangedDelegate, UItemHandle* /*ItemHandle*/, bool /*bContinueRecursively*/);
DECLARE_DELEGATE_OneParam(FOnToolDataLoadedDelegate, ELightControlLoadingResult /*LoadingResult*/)


DECLARE_DELEGATE_OneParam(FOnToolDataTransacted, const FTransactionObjectEvent&);
DECLARE_DELEGATE_TwoParams(FPostLightTransacted, const FTransactionObjectEvent&, UBaseLight&);
DECLARE_DELEGATE_OneParam(FMetaDataExtension, TSharedPtr<FJsonObject> /*RootJsonObject*/)

// Top UObject that is responsible for the hierarchy of Item Handles and lights
// Needed in order for the editor's transaction system to affect drag-drop operations

UCLASS(BlueprintType)
class CRADLELIGHTCONTROL_API UToolData : public UObject
{
    GENERATED_BODY()
public:

    UToolData();

    ~UToolData();

    UFUNCTION(BlueprintPure)
    UBaseLight* GetLightByName(FString Name);
#if WITH_EDITOR
    void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
#endif
    void ClearAllData();
    // Adds an empty item handle. Must be filled out by the invoker.
    UBaseLight* AddItem(bool bAssignId = true);
    
    void BeginTransaction();

    TSharedPtr<FJsonObject> SaveStateToJson(FString Path, bool bUpdatePresetPath = true); // Saves the state of the hierarchy to a JSON file.
    TSharedPtr<FJsonObject> LoadStateFromJSON(FString Path, bool bUpdatePresetPath = true); // Loads the state of the hierarchy from a JSON file.

    void AutoSave(); // Called periodically or when certain events happen in the editor, like it shutting down.

    TSharedPtr<FJsonObject> OpenMetaDataJson(); // Returns the MetaData JSON object.

    void SaveMetaData();
    void LoadMetaData();

    FString DataName; // A decorative name of the data set. Used to determine name for its files and in its hierarchy widget.

    bool bCurrentlyLoading;
    FString ToolPresetPath; // The last used path when saving or loading a preset. If empty, will resort to an autosave file.

    FMetaDataExtension MetaDataSaveExtension; // Possible extension to how the metadata is being saved.
    FMetaDataExtension MetaDataLoadExtension; // Possible extension to how the metadata is being loaded.
    
    FOnTreeStructureChangedDelegate TreeStructureChangedDelegate; // Callback for whenever the structure of the tree hierarchy changes. Updates the relevant slate widget.
    FItemExpansionChangedDelegate ItemExpansionChangedDelegate; // Callback for when an item's expansion is changed.
    FOnToolDataLoadedDelegate OnToolDataLoaded; // Callback when the tool data is loaded. Used to generate the necessary widgets for the items.

    FOnToolDataTransacted OnTransacted;
    FPostLightTransacted PostLightTransacted;


    // Controls what type of lights the ToolData works with. UVirtualLight::StaticClass() or UDMXLight::StaticClass().
    UPROPERTY(NonTransactional)
        UClass* ItemClass;

    int32 LightIdCounter;

    //// List of all the root items in the dataset. Each root item may or may not have children.
    //UPROPERTY()
    //    TArray<UItemHandle*> RootItems;

    //// 1D list of all items in the tree, no matter if they are parented or not
    //UPROPERTY()
    //    TArray<UItemHandle*> ListOfTreeItems;
    //
    //// 1D list of all handles which hold lights items as opposed to being groups.
    //UPROPERTY()
    //    TArray<UItemHandle*> ListOfLightItems;

    UPROPERTY()
        TArray<UBaseLight*> Lights;
};
