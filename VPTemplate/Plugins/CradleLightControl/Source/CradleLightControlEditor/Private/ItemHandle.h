#pragma once

#include "CoreMinimal.h"

#include "Json.h"

#include "Slate.h"

#include "LightControlLoadingResult.h"

#include "ItemHandle.generated.h"

// Enum of the types of possible lights in the tool
// This is made to work together with the EIconType enum from the editor module, so they need to be changed together

class FItemDragDropOperation : public FDragDropOperation
{
public:

    TArray<class UItemHandle*> DraggedItems;
    UItemHandle* Destination;
};

// UObject which is responsible for storing the UBaseLights in a tool's hierarchy
// Represents only information that is relevant for the tool that uses it


UCLASS(BlueprintType)
class UItemHandle : public UObject
{
    GENERATED_BODY()

public:
    UItemHandle()
        : Parent(nullptr)
        , Name("Unnamed")
        , Note("")
        , Item(nullptr)
		, LightId(-1)
    {
        SetFlags(GetFlags() | RF_Transactional);
    };

    

    // Used for checkboxes because checkboxes have 3 states
    ECheckBoxState IsLightEnabled() const;
    void EnableGrouped(bool NewState);

    // Check if the attempted drag drop operation is valid.
	// An invalid operation is trying to drag a parent into one of its children
    static bool VerifyDragDrop(UItemHandle* Dragged, UItemHandle* Destination);
    // Check if this item handle has the given item handle as an indirect child
    bool HasAsIndirectChild(UItemHandle* Item);

    TSharedPtr<FJsonValue> SaveToJson();

    ELightControlLoadingResult LoadFromJson(TSharedPtr<FJsonObject> JsonObject);

    // Returns all lights under this handle, including children of children
    void GetLights(TArray<UItemHandle*>& Array);

    void UpdateFolderIcon();


    // Returns the number of lights under this handle. Includes children of children.
    int LightCount() const;

    void BeginTransaction(bool bAffectItem = true, bool bAffectParent = false);
#if WITH_EDITOR
    virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
#endif
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

    UPROPERTY()
        int32 LightId;

    // Reference to the ToolData instance which has created and owns the handle
    class UEditorData* EditorData;

   
};
