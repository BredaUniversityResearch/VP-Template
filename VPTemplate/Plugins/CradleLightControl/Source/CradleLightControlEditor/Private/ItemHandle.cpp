#include "ItemHandle.h"

#include "VirtualLight.h"
#include "EditorData.h"

#include "Engine.h"

#include "CradleLightControl.h"
#include "ToolData.h"

ECheckBoxState UItemHandle::IsLightEnabled() const
{
    bool AllOff = true, AllOn = true;

    if (Item)
    {
        // If the item is not a group, we just check if the item is On or Off and decide based on that
        return Item->IsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }

    // If the item is a group, we need to see the state of all its children to decide on the state
    for (auto& Child : Children)
    {
        auto State = Child->IsLightEnabled();
        if (State == ECheckBoxState::Checked)
            AllOff = false;
        else if (State == ECheckBoxState::Unchecked)
            AllOn = false;
        else if (State == ECheckBoxState::Undetermined)
            return ECheckBoxState::Undetermined;

        // Are there both On and Off lights? Then we go with undetermined
        if (!AllOff && !AllOn)
            return ECheckBoxState::Undetermined;
    }

    if (AllOn)
        return ECheckBoxState::Checked;
    else
        return ECheckBoxState::Unchecked;


}

void UItemHandle::EnableGrouped(bool NewState)
{
    if (Item)
    {
        Item->SetEnabled(NewState);
    }
    else
    {
        // Change the state of all children if this is a group, recursively
        for (auto& Child : Children)
        {
            Child->EnableGrouped(NewState);
        }
    }
}

bool UItemHandle::VerifyDragDrop(UItemHandle* Dragged, UItemHandle* Destination)
{
    // Would result in the child and parent creating a circle
    if (Dragged->Children.Find(Destination) != INDEX_NONE)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Cannot drag parent to child");
        return false;
    }

    // Can't drag the item on itself, can we now
    if (Dragged == Destination)
    {
        return false;
    }


    // Would cause a circular dependency between the tree items
    if (Dragged->HasAsIndirectChild(Destination))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Cannot drag parent to indirect child");
        return false;
    }

    // No need to do anything in this case
    if (Destination->Children.Find(Dragged) != INDEX_NONE)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Drag drop cancelled");
        return false;
    }

    return true;
}

bool UItemHandle::HasAsIndirectChild(UItemHandle* ItemHandle)
{
    if (Children.Find(ItemHandle) != INDEX_NONE)
        return true;

    // Check the children recursively
    for (auto TreeItem : Children)
    {
        if (TreeItem->HasAsIndirectChild(ItemHandle))
            return true;
    }

    return false;
}


TSharedPtr<FJsonValue> UItemHandle::SaveToJson()
{
    TSharedPtr<FJsonObject> JsonItem = MakeShared<FJsonObject>();


    int ItemState = Item->bIsEnabled;

    JsonItem->SetStringField("Name", Name);
    JsonItem->SetStringField("Note", Note);
	if (!Item)
    {
        // Otherwise we save all of its child handles
        TArray<TSharedPtr<FJsonValue>> ChildrenJson;

        for (auto Child : Children)
        {
            ChildrenJson.Add(Child->SaveToJson());
        }

        JsonItem->SetArrayField("Children", ChildrenJson);
        JsonItem->SetNumberField("Light Id", -1);
    }
    else
    {
    	JsonItem->SetNumberField("Light Id", LightId);
    }

    TSharedPtr<FJsonValue> JsonValue = MakeShared<FJsonValueObject>(JsonItem);
    return JsonValue;
}


ELightControlLoadingResult UItemHandle::LoadFromJson(TSharedPtr<FJsonObject> JsonObject)
{
    Name = JsonObject->GetStringField("Name");
    Note = JsonObject->GetStringField("Note");
    LightId = JsonObject->GetNumberField("Light Id");
    if (LightId != -1)
    {
        auto Light = EditorData->GetToolData()->Lights.FindByPredicate([this](UBaseLight* Light)
        {
                return Light->Id == LightId;
        });
        Item = Light ? *Light : nullptr;
        if (!GWorld)
        {
            UE_LOG(LogCradleLightControl, Error, TEXT("There was an error with the engine. Try loading again. If the issue persists, restart the engine."));
            return ELightControlLoadingResult::EngineError;
        }

    }
    else
    {
        auto JsonChildren = JsonObject->GetArrayField("Children");

        auto ChildrenLoadingSuccess = ELightControlLoadingResult::Success;
        for (auto Child : JsonChildren)
        {
            const TSharedPtr<FJsonObject>* ChildObjectPtr;
            auto Success = Child->TryGetObject(ChildObjectPtr);
            check(Success);

            auto ChildObject = *ChildObjectPtr;
            int ChildType = ChildObject->GetNumberField("Type");
            auto ChildItem = EditorData->AddItem();

            ChildItem->Parent = this;

            auto ChildResult = ChildItem->LoadFromJson(ChildObject);
            if (ChildResult != ELightControlLoadingResult::Success)
            {
                if (ChildrenLoadingSuccess == ELightControlLoadingResult::Success)
                {
                    ChildrenLoadingSuccess = ChildResult;
                }
                else
                    ChildrenLoadingSuccess = ELightControlLoadingResult::MultipleErrors;
            }
            Children.Add(ChildItem);
        }
        return ChildrenLoadingSuccess;
    }


    return ELightControlLoadingResult::Success;
}

void UItemHandle::GetLights(TArray<UItemHandle*>& Array)
{
    if (!Item)
    {
        for (auto& Child : Children)
            Child->GetLights(Array);
    }
    else
    {
        Array.Add(this);
    }
}

void UItemHandle::UpdateFolderIcon()
{
    if (Item)
        return;
    TArray<UItemHandle*> ChildLights;
    GetLights(ChildLights);

    auto IconType = Item ? Item->Type : Mixed;

    if (ChildLights.Num() > 0)
    {
        IconType = ChildLights[0]->Item->Type;

        for (size_t i = 1; i < ChildLights.Num(); i++)
        {
            if (IconType != ChildLights[i]->Item->Type)
            {
                IconType = Mixed;
                break;
            }
        }
    }
    else
        IconType = Mixed;


    if (Parent)
        Parent->UpdateFolderIcon();
}


int UItemHandle::LightCount() const
{
    if (Item)
    {
        return 1;
    }
    auto LightCount = 0;

    for (auto Child : Children)
    {
        LightCount += Child->LightCount();
    }

    return LightCount;
}

void UItemHandle::BeginTransaction(bool bAffectItem, bool bAffectParent)
{
    Modify();
    if (bAffectItem && Item)
    {
        Item->BeginTransaction();
    }

    if (bAffectParent && Parent)
    {
        Parent->BeginTransaction(false);
    }
}

#if WITH_EDITOR
void UItemHandle::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
    UObject::PostTransacted(TransactionEvent);
    // Transactions with Item Handles may involve changes in the tree hierarchy, so we need to notify the tree widget about it
    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
    {
        if (Item)
            EditorData->TreeStructureChangedDelegate.ExecuteIfBound();

    }
}
#endif