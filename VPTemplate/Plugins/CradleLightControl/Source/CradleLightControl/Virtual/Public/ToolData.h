#pragma once

#include "CoreMinimal.h"

#include "ToolData.generated.h"

class UItemHandle;

UENUM()
enum EIconType
{
    GeneralLightOff = 0,
    GeneralLightOn,
    GeneralLightUndetermined,
    SkyLightOff,
    SkyLightOn,
    SkyLightUndetermined,
    SpotLightOff,
    SpotLightOn,
    SpotLightUndetermined,
    DirectionalLightOff,
    DirectionalLightOn,
    DirectionalLightUndetermined,
    PointLightOff,
    PointLightOn,
    PointLightUndetermined,
    FolderClosed,
    FolderOpened
};

DECLARE_DELEGATE(FClearSelectionDelegate);
DECLARE_DELEGATE_RetVal_TwoParams(FString, FLightJsonFileDialogDelegate, FString /*Title*/, FString /*StartDir*/);
DECLARE_DELEGATE(FOnTreeStructureChangedDelegate);
DECLARE_DELEGATE_TwoParams(FItemExpansionChangedDelegate, UItemHandle*, bool);
DECLARE_DELEGATE_OneParam(FOnMasterLightTransactedDelegate, UItemHandle*);

UCLASS()
class UToolData : public UObject
{
    GENERATED_BODY()
public:

    UToolData();

    ~UToolData();

    void GenerateIcons();
    FCheckBoxStyle MakeCheckboxStyleForType(uint8 IconType);
    FSlateBrush& GetIcon(EIconType Icon);

    void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;

    TSharedPtr<STreeView<UItemHandle*>> GetWidget();

    void ClearAllData();
    UItemHandle* AddItem(bool bIsFolder = false);

    bool IsAMasterLightSelected();
    bool IsSingleGroupSelected();
    bool MultipleItemsSelected();
    bool MultipleLightsInSelection();
    UItemHandle* GetMasterLight();
    UItemHandle* GetSelectedGroup();
    UItemHandle* GetSingleSelectedItem();
    const TArray<UItemHandle*>& GetSelectedLights();

    void BeginTransaction();

    FReply SaveCallBack();
    FReply SaveAsCallback();
    void SaveStateToJson(FString Path, bool bUpdatePresetPath = true);
    FReply LoadCallBack();
    void LoadStateFromJSON(FString Path, bool bUpdatePresetPath = true);

    void AutoSave();

    void SaveMetaData();
    void LoadMetaData();

    FString DataName;

    bool bCurrentlyLoading;
    FString ToolPresetPath;
    
    FClearSelectionDelegate ClearSelectionDelegate;
    FLightJsonFileDialogDelegate SaveFileDialog;
    FLightJsonFileDialogDelegate OpenFileDialog;

    FOnTreeStructureChangedDelegate TreeStructureChangedDelegate;
    FItemExpansionChangedDelegate ItemExpansionChangedDelegate;
    FOnMasterLightTransactedDelegate MasterLightTransactedDelegate;

    FTimerHandle AutoSaveTimer;
    //TSharedPtr<FActiveTimerHandle> AutoSaveTimer;


    UPROPERTY(NonTransactional)
        UClass* ItemClass;

    UPROPERTY()
        TArray<UItemHandle*> RootItems;

    UPROPERTY()
        TArray<UItemHandle*> ListOfTreeItems;


    UPROPERTY()
        TArray<UItemHandle*> SelectedItems;

    UPROPERTY()
        TArray<UItemHandle*> LightsUnderSelection;
    UPROPERTY()
        UItemHandle* SelectionMasterLight;

    UPROPERTY()
        TArray<UItemHandle*> ListOfLightItems;

    UPROPERTY(NonTransactional)
        TMap<TEnumAsByte<EIconType>, FSlateBrush> Icons;
};
