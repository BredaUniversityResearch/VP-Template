#pragma once

#include "CoreMinimal.h"

#include "Json.h"

#include "ItemHandle.generated.h"

// Enum of the types of possible lights in the tool
// This is made to work together with the EIconType enum from the editor module, so they need to be changed together
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

    TArray<class UItemHandle*> DraggedItems;
    UItemHandle* Destination;
};

// UObject which is responsible for storing the UBaseLights in a tool's hierarchy
// Represents only information that is relevant for the tool that uses it


UCLASS(BlueprintType)
class CRADLELIGHTCONTROL_API UItemHandle : public UObject
{
    GENERATED_BODY()

public:
    UItemHandle()
        : Parent(nullptr)
        , Name("Unnamed")
        , Note("")
        , Item(nullptr)
    {
        SetFlags(GetFlags() | RF_Transactional);
    };

    // For use with the virtual light control tool. Used when starting a play session to update the light actor references.
    // This is necessary because when a play session is started, levels are duplicated, so all actor references are technically invalid in the context of the play session.
    // Takes an array of AActor*, for ease of use with UGameplayStatics::GetAllActorsOfClass();
    void UpdateVirtualLights(TArray<AActor*>& ActorLights);

    // FOr use with the virtual light control tool. Used when a play session is ended and the engine returns to the level editor.
    // Necessary in order to restore the light actor pointers in the tool to the ones from the original level.
    void RestoreVirtualLightReferences();

    // Used for checkboxes because checkboxes have 3 states
    ECheckBoxState IsLightEnabled() const;
    void OnCheck(ECheckBoxState NewState);

    // Check if the attempted drag drop operation is valid.
	// An invalid operation is trying to drag a parent into one of its children
    static bool VerifyDragDrop(UItemHandle* Dragged, UItemHandle* Destination);
    // Check if this item handle has the given item handle as an indirect child
    bool HasAsIndirectChild(UItemHandle* Item);

    TSharedPtr<FJsonValue> SaveToJson();
    enum ELoadingResult : uint8
    {
        Success = 0,
        InvalidType,
        LightNotFound,
        EngineError,
        MultipleErrors
    };

    ELoadingResult LoadFromJson(TSharedPtr<FJsonObject> JsonObject);
    // Update the handle's expansion in the hierarchy it belongs to
    FReply RemoveFromTree();

    // Returns all lights under this handle, including children of children
    void GetLights(TArray<UItemHandle*>& Array);

    void UpdateFolderIcon();


    // Returns the number of lights under this handle. Includes children of children.
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

    // The type of the data the handle represents. If it's not a Folder, it represent a light item.
    TEnumAsByte<ETreeItemType> Type;

    // Reference to the ToolData instance which has created and owns the handle
    class UToolData* ToolData;

    //// Top widget which contains all other widgets for the handle's widget
    //TSharedPtr<SBox> TableRowBox;

    //TSharedPtr<SCheckBox> StateCheckbox;
    //// SBox containing the widget for the name of the handle, or editable text if it is being renamed
    //TSharedPtr<SBox> RowNameBox;
    //FCheckBoxStyle CheckBoxStyle;


    //bool bExpanded;
    //bool bMatchesSearchString;

    //bool bInRename;
};
