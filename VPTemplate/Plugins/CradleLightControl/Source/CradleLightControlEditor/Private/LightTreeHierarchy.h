#pragma once

#include "Templates/SharedPointer.h"
#include "Chaos/AABB.h"
#include "Slate.h"
#include "DMXProtocolCommon.h"
#include "LightControlLoadingResult.h"


class UItemHandle;
class UToolData;

DECLARE_DELEGATE_OneParam(FUpdateItemDataDelegate, class UBaseLight*)
DECLARE_DELEGATE(FItemDataVerificationDelegate);
DECLARE_DELEGATE(FTreeSelectionChangedDelegate);


class SLightHierarchyWidget : public SCompoundWidget
{
public:


    SLATE_BEGIN_ARGS(SLightHierarchyWidget)
        : _Name("Unnamed tree view")
    {}

    SLATE_ARGUMENT(class UEditorData*, EditorData)

    SLATE_ARGUMENT(FString, Name)

    SLATE_ARGUMENT(FUpdateItemDataDelegate, DataUpdateDelegate)

    SLATE_ARGUMENT(FItemDataVerificationDelegate, DataVerificationDelegate)
    SLATE_ARGUMENT(float, DataVerificationInterval)

    SLATE_ARGUMENT(FTreeSelectionChangedDelegate, SelectionChangedDelegate)

    SLATE_END_ARGS()

    void Construct(const FArguments& Args);
    void PreDestroy();

    void OnActorSpawned(AActor* Actor);


    void BeginTransaction();

    void GenerateWidgetForItem(UItemHandle* Item);

    TSharedRef<ITableRow> AddToTree(::UItemHandle* Item, const TSharedRef<STableViewBase>& OwnerTable);

    void GetTreeItemChildren(::UItemHandle* Item, TArray<UItemHandle*>& Children);
    void SelectionCallback(UItemHandle* Item, ESelectInfo::Type SelectType);
    FReply AddFolderToTree();
    void OnToolDataLoadedCallback(ELightControlLoadingResult LoadingResult);
    void RegenerateItemHandleWidgets(UItemHandle* ItemHandle);
    EActiveTimerReturnType VerifyLights(double, float);

    FReply DragDropBegin(const FGeometry& Geometry, const FPointerEvent& MouseEvent);
    FReply DragDropEnd(const FDragDropEvent& DragDropEvent);

    void SearchBarOnChanged(const FText& NewString);

    TSharedPtr<class STreeItemWidget> GetWidgetForItem(UItemHandle* ItemHandle);
    void UpdateExpansionForItem(UItemHandle* ItemHandle, bool bContinueRecursively = true);
    void ExpansionChangedCallback(UItemHandle* ItemHandle, bool bNewState);
    void ChangeExpansionInTree(UItemHandle* ItemHandle, bool bNewState);

    FText GetPresetFilename() const;


    UEditorData* EditorData;
    TMap<UItemHandle*, TSharedPtr<STreeItemWidget>> ItemWidgets;

    FSlateIcon SaveIcon;
    FSlateIcon SaveAsIcon;
    FSlateIcon LoadIcon;

    FText HeaderText;


    TSharedPtr<STreeView<UItemHandle*>> Tree;
    FString SearchString;

    FUpdateItemDataDelegate DataUpdateDelegate;
    FItemDataVerificationDelegate DataVerificationDelegate;

    //class UToolData* TransactionalVariables;
    TSharedPtr<FActiveTimerHandle> LightVerificationTimer;

    FTreeSelectionChangedDelegate SelectionChangedDelegate;
};
