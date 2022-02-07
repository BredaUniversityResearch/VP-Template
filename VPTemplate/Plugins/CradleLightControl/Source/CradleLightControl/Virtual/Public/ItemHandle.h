#pragma once

#include "CoreMinimal.h"

#include "Json.h"

#include "ItemHandle.generated.h"

UENUM()
enum ETreeItemType
{
    Folder = 0,
    Mixed = Folder,
    SkyLight,
    SpotLight,
    DirectionalLight,
    PointLight,
    Invalid
};


class FItemDragDropOperation : public FDragDropOperation
{
public:

    class UItemHandle* DraggedItem;
};

UCLASS()
class UItemHandle : public UObject
{
    GENERATED_BODY()

public:
    UItemHandle()
        : Parent(nullptr)
        , Name("Unnamed")
        , Note("")
        , Item(nullptr)
        , bExpanded(false)
        , bMatchesSearchString(true)
    {
        SetFlags(GetFlags() | RF_Transactional);
    };

    // Used for checkboxes because checkboxes have 3 states
    ECheckBoxState IsLightEnabled() const;
    void OnCheck(ECheckBoxState NewState);

    FReply TreeDragDetected(const FGeometry& Geometry, const FPointerEvent& MouseEvent);
    FReply TreeDropDetected(const FDragDropEvent& DragDropEvent);

    void GenerateTableRow();

    static bool VerifyDragDrop(UItemHandle* Dragged, UItemHandle* Destination);
    bool HasAsIndirectChild(UItemHandle* Item);

    FReply StartRename(const FGeometry&, const FPointerEvent&);
    void EndRename(const FText& Text, ETextCommit::Type CommitType);

    TSharedPtr<FJsonValue> SaveToJson();
    enum ELoadingResult
    {
        Success = 0,
        InvalidType,
        LightNotFound,
        EngineError,
        MultipleErrors
    };

    ELoadingResult LoadFromJson(TSharedPtr<FJsonObject> JsonObject);
    void ExpandInTree();
    FReply RemoveFromTree();

    void GetLights(TArray<UItemHandle*>& Array);

    void UpdateFolderIcon();

    bool CheckNameAgainstSearchString(const FString& SearchString);

    int LightCount() const;

    void BeginTransaction(bool bAffectItem = true, bool bAffectParent = false);
    virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;

    UPROPERTY()
        TArray<UItemHandle*> Children;

    UPROPERTY()
        UItemHandle* Parent;

    UPROPERTY()
        FString Name;

    UPROPERTY()
        FString Note;

    UPROPERTY()
        class UBaseLight* Item;

    TEnumAsByte<ETreeItemType> Type;

    class UToolData* ToolData;

    TSharedPtr<SBox> TableRowBox;

    TSharedPtr<SCheckBox> StateCheckbox;
    TSharedPtr<SBox> RowNameBox;
    FCheckBoxStyle CheckBoxStyle;


    bool bExpanded;
    bool bMatchesSearchString;

private:


    bool bInRename;
};
