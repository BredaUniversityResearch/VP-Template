#include "ItemHandle.h"

#include "VirtualLight.h"
#include "ToolData.h"

#include "CradleLightControl.h"


void UItemHandle::UpdateVirtualLights(TArray<AActor*>& ActorLights)
{
    auto VLight = Cast<UVirtualLight>(Item);
    if (VLight && VLight->ActorPtr)
    {
        auto LightName = VLight->ActorPtr->GetName();
        for (auto& L : ActorLights)
        {
            if (LightName == L->GetName())
            {
                VLight->OriginalActor = VLight->ActorPtr;
                VLight->ActorPtr = L;
                break;
            }
        }
    }
    else
    {
	    for (auto& Child : Children)
	    {
            Child->UpdateVirtualLights(ActorLights);
	    }
    }
}

void UItemHandle::RestoreVirtualLightReferences()
{
    auto VLight = Cast<UVirtualLight>(Item);
    if (VLight && VLight->ActorPtr)
    {
    	VLight->ActorPtr = VLight->OriginalActor;
    }
    else
    {
        for (auto& Child : Children)
        {
            Child->RestoreVirtualLightReferences();
        }
    }
}

ECheckBoxState UItemHandle::IsLightEnabled() const
{
    bool AllOff = true, AllOn = true;

    if (Type != Folder)
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

void UItemHandle::OnCheck(ECheckBoxState NewState)
{
    bool B = false;
    if (NewState == ECheckBoxState::Checked)
        B = true;

    GEditor->BeginTransaction(FText::FromString(Name + " State change"));

    if (Type != Folder)
    {
		Item->SetEnabled(B);	    
    }
    else
    {
        // Change the state of all children if this is a group, recursively
	    for (auto& Child : Children)
	    {
            Child->OnCheck(NewState);
	    }
    }

    GEditor->EndTransaction();
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
    JsonItem->SetNumberField("Type", Type);
    if (Type != Folder)
    {
        // If this is not a group, we save the item held by the handle
        JsonItem->SetObjectField("Item", Item->SaveAsJson());    
    }
    else
    {
        // Otherwise we save all of its child handles
        TArray<TSharedPtr<FJsonValue>> ChildrenJson;

        for (auto Child : Children)
        {
            ChildrenJson.Add(Child->SaveToJson());
        }

        JsonItem->SetArrayField("Children", ChildrenJson);

    }

    TSharedPtr<FJsonValue> JsonValue = MakeShared<FJsonValueObject>(JsonItem);
    return JsonValue;
}


UItemHandle::ELoadingResult UItemHandle::LoadFromJson(TSharedPtr<FJsonObject> JsonObject)
{
    Name = JsonObject->GetStringField("Name");
    Note = JsonObject->GetStringField("Note");
    Type = StaticCast<ETreeItemType>(JsonObject->GetNumberField("Type"));
    auto JsonItem = JsonObject->GetObjectField("Item");
    if (Type != Folder)
    {
        if (!GWorld)
        {
            UE_LOG(LogCradleLightControl, Error, TEXT("There was an error with the engine. Try loading again. If the issue persists, restart the engine."));
            return EngineError;
        }

        Item->LoadFromJson(JsonItem);
    }
    else
    {
        auto JsonChildren = JsonObject->GetArrayField("Children");

        auto ChildrenLoadingSuccess = Success;
        for (auto Child : JsonChildren)
        {
            const TSharedPtr<FJsonObject>* ChildObjectPtr;
            auto Success = Child->TryGetObject(ChildObjectPtr);
            check(Success);

            auto ChildObject = *ChildObjectPtr;
            int ChildType = ChildObject->GetNumberField("Type");
            auto ChildItem = ToolData->AddItem(ChildType == Folder); 

            ChildItem->Parent = this;

            auto ChildResult = ChildItem->LoadFromJson(ChildObject);
            if (ChildResult != ELoadingResult::Success)
            {
                if (ChildrenLoadingSuccess == ELoadingResult::Success)
                {
                    ChildrenLoadingSuccess = ChildResult;
                }
                else
                    ChildrenLoadingSuccess = ELoadingResult::MultipleErrors;
            }
            Children.Add(ChildItem);
        }
        return ChildrenLoadingSuccess;
    }


    return Success;
}

FReply UItemHandle::RemoveFromTree()
{
    GEditor->BeginTransaction(FText::FromString("Delete Light control folder"));
    BeginTransaction(false);
    if (Parent)
    {
        // If this handle has a parent, we move all of its children to the parent
        Parent->BeginTransaction(false);
        for (auto Child : Children)
        {
            Child->BeginTransaction(false);
            Child->Parent = Parent;
            Parent->Children.Add(Child);

        }
        Parent->Children.Remove(this);
    }
    else
    {
        // If the handle is a root item, we make its children root items as well
        ToolData->BeginTransaction();
        for (auto Child : Children)
        {
            Child->BeginTransaction(false);
            Child->Parent = nullptr;
            ToolData->RootItems.Add(Child);
        }
        ToolData->RootItems.Remove(this);
    }
    GEditor->EndTransaction();
    Children.Empty();
    ToolData->TreeStructureChangedDelegate.ExecuteIfBound();

    return FReply::Handled();
}

void UItemHandle::GetLights(TArray<UItemHandle*>& Array)
{
    if (Type == Folder)
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
    if (Type != Folder)
        return;
    TArray<UItemHandle*> ChildLights;
    GetLights(ChildLights);

    auto IconType = Type;

    if (ChildLights.Num() > 0)
    {
        IconType = ChildLights[0]->Type;

        for (size_t i = 1; i < ChildLights.Num(); i++)
        {
            if (IconType != ChildLights[i]->Type)
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
    if (Type != Folder)
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
    if (bAffectItem)
    {
        Item->BeginTransaction();
    }

    if (bAffectParent && Parent)
    {
        Parent->BeginTransaction(false);
    }
}

void UItemHandle::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
    UObject::PostTransacted(TransactionEvent);
    // Transactions with Item Handles may involve changes in the tree hierarchy, so we need to notify the tree widget about it
    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
    {
        if (Type == Folder)
            ToolData->TreeStructureChangedDelegate.ExecuteIfBound();
        
    }
}
