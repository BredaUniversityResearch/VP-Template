#include "ToolData.h"

#include "ItemHandle.h"

#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"

#include "BaseLight.h"

//#include "LightTreeHierarchy.h"

#include "Interfaces/IPluginManager.h"

UToolData::UToolData()
{
    SetFlags(GetFlags() | RF_Transactional);


}

UToolData::~UToolData()
{
    //AutoSave();
}

UBaseLight* UToolData::GetLightByName(FString Name)
{
	for (auto& LightItem : ListOfLightItems)
	{
		if (LightItem->Name == Name)
		{
            return LightItem->Item;
		}
	}

	if (GEngine)
	{
        GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, FString::Printf(TEXT("Could not find item with name \"%s\""), *Name));
	}

    return nullptr;
}


void UToolData::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{

    if (TransactionEvent.GetEventType() == ETransactionObjectEventType::UndoRedo)
    {
        volatile auto Master = GetMasterLight();
        TreeStructureChangedDelegate.ExecuteIfBound();
        check(Master == GetMasterLight());
    }
}


bool UToolData::IsAMasterLightSelected()
{
    return IsValid(GetMasterLight());
}

bool UToolData::MultipleItemsSelected()
{
    return SelectedItems.Num() > 1;
}

bool UToolData::MultipleLightsInSelection()
{
    return LightsUnderSelection.Num() > 1;
}

UItemHandle* UToolData::GetMasterLight()
{
    return SelectionMasterLight;
}

UItemHandle* UToolData::GetSelectedGroup()
{
    if (SelectedItems.Num() && SelectedItems[0]->Type == Folder)
    {
        return SelectedItems[0];
    }
    return nullptr;
}

UItemHandle* UToolData::GetSingleSelectedItem()
{
    if (IsSingleGroupSelected())
        return GetSelectedGroup();
    if (IsAMasterLightSelected())
        return GetMasterLight();
    return nullptr;
}

const TArray<UItemHandle*>& UToolData::GetSelectedLights()
{
    return LightsUnderSelection;
}

TArray<UItemHandle*> UToolData::GetSelectedItems()
{
    return SelectedItems;
}

void UToolData::BeginTransaction()
{
    Modify();
}


void UToolData::ClearAllData()
{
    RootItems.Empty();
    ListOfTreeItems.Empty();
    SelectedItems.Empty();
    LightsUnderSelection.Empty();
    ListOfLightItems.Empty();

    SelectionMasterLight = nullptr;

    ClearSelectionDelegate.ExecuteIfBound();
}

UItemHandle* UToolData::AddItem(bool bIsFolder)
{
    auto Item = NewObject<UItemHandle>();
    Item->ToolData = this;
    Item->Parent = nullptr;

    ListOfTreeItems.Add(Item);

    //TreeRootItems.Add(Item);
    if (bIsFolder)
    {
        Item->Type = Folder;
    }
    else // Do this so that only actual lights which might be deleted in the editor are checked for validity
    {
        ListOfLightItems.Add(Item);
        Item->Item = NewObject<UBaseLight>(this, ItemClass);
        Item->Item->Handle = Item;
    }

    return Item;
}


bool UToolData::IsSingleGroupSelected()
{
    return SelectedItems.Num() == 1 && SelectedItems[0]->Type == Folder;
}


FReply UToolData::SaveCallBack()
{
    if (ToolPresetPath.IsEmpty())
    {
        return SaveAsCallback();
    }
    else
        SaveStateToJson(ToolPresetPath);

    return FReply::Handled();
}


FReply UToolData::SaveAsCallback()
{
    check(SaveFileDialog.IsBound());


    FString StartPath;

    if (ToolPresetPath.IsEmpty())
    {
        auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
        StartPath = ThisPlugin->GetContentDir();
    }
    else
        StartPath = ToolPresetPath;


    auto Path = SaveFileDialog.Execute("Select file to save tool state to", StartPath);
    if (!Path.IsEmpty())
    {
        SaveStateToJson(Path);
    }
    return FReply::Handled();
}

void UToolData::SaveStateToJson(FString Path, bool bUpdatePresetPath)
{
    TArray<TSharedPtr<FJsonValue>> TreeItemsJSON;

    for (auto TreeItem : RootItems)
    {
        TreeItemsJSON.Add(TreeItem->SaveToJson());
    }
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

    RootObject->SetArrayField("TreeElements", TreeItemsJSON);

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    FFileHelper::SaveStringToFile(Output, *Path);
    if (bUpdatePresetPath)
        ToolPresetPath = Path;
}

FReply UToolData::LoadCallBack()
{
    check(OpenFileDialog.IsBound());

    FString StartPath;

    if (ToolPresetPath.IsEmpty())
    {
        auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
        StartPath = ThisPlugin->GetContentDir();
    }
    else
        StartPath = ToolPresetPath;

    auto Path = OpenFileDialog.Execute("Select file to load tool state from", StartPath);
    if (!Path.IsEmpty())
    {
        LoadStateFromJSON(Path);
    }
    return FReply::Handled();
}

void UToolData::LoadStateFromJSON(FString Path, bool bUpdatePresetPath)
{
    bCurrentlyLoading = true;

    _ASSERT(Widget);
    _ASSERT(ClearSelectionDelegate.IsBound());

    FString Input;
    ClearSelectionDelegate.ExecuteIfBound();
    if (FFileHelper::LoadFileToString(Input, *Path))
    {
        if (bUpdatePresetPath)
            ToolPresetPath = Path;
        UE_LOG(LogTemp, Display, TEXT("Beginning light control tool state loading from %s"), *Path);
        RootItems.Empty();
        ListOfTreeItems.Empty();
        ListOfLightItems.Empty();
        TSharedPtr<FJsonObject> JsonRoot;
        TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Input);
        FJsonSerializer::Deserialize(JsonReader, JsonRoot);

        auto LoadingResult = UItemHandle::ELoadingResult::Success;
        for (auto TreeElement : JsonRoot->GetArrayField("TreeElements"))
        {
            const TSharedPtr<FJsonObject>* TreeElementObjectPtr;
            auto Success = TreeElement->TryGetObject(TreeElementObjectPtr);
            auto TreeElementObject = *TreeElementObjectPtr;
            _ASSERT(Success);
            int Type = TreeElementObject->GetNumberField("Type");
            auto Item = AddItem(Type == 0); // If Type is 0, this element is a folder, so we add it as a folder
            auto Res = Item->LoadFromJson(TreeElementObject);

            if (Res != UItemHandle::ELoadingResult::Success)
            {
                if (LoadingResult == UItemHandle::ELoadingResult::Success)
                {
                    LoadingResult = Res;
                }
                else LoadingResult = UItemHandle::ELoadingResult::MultipleErrors;
            }

            RootItems.Add(Item);
        }
        TreeStructureChangedDelegate.ExecuteIfBound();

        for (auto TreeItem : RootItems)
        {
            TreeItem->ExpandInTree();
        }
        OnToolDataLoaded.ExecuteIfBound(LoadingResult);
     
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Could not open file %s"), *Path);
        ToolPresetPath = "";
    }

    bCurrentlyLoading = false;
}


void UToolData::AutoSave()
{
    UE_LOG(LogTemp, Display, TEXT("Autosaving light control tool state."));

    if (ToolPresetPath.IsEmpty())
    {
        auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
        auto SavedDir = ThisPlugin->GetBaseDir() + "/Saved";

        SaveStateToJson(SavedDir + "/" + DataName + "AutoSave.json", false);
    }
    else
        SaveStateToJson(ToolPresetPath, false);

    SaveMetaData();
}

TSharedPtr<FJsonObject> UToolData::OpenMetaDataJson()
{
    auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
    auto SavedDir = ThisPlugin->GetBaseDir() + "/Saved";
    FString Input;
    if (FFileHelper::LoadFileToString(Input, *(SavedDir + "/" + DataName + "MetaData.json")))
    {
        TSharedPtr<FJsonObject> JsonRoot;
        TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Input);
        FJsonSerializer::Deserialize(JsonReader, JsonRoot);

        return JsonRoot;
    }

    return nullptr;

}

void UToolData::SaveMetaData()
{
    UE_LOG(LogTemp, Display, TEXT("Saving light control meta data."));
    auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
    auto SavedDir = ThisPlugin->GetBaseDir() + "/Saved";

    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

    RootObject->SetStringField("LastUsedPreset", ToolPresetPath);

    MetaDataSaveExtension.ExecuteIfBound(RootObject);

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    FFileHelper::SaveStringToFile(Output, *(SavedDir + "/" + DataName + "MetaData.json"));
}

void UToolData::LoadMetaData()
{

    UE_LOG(LogTemp, Display, TEXT("Loading light control meta data."));
    auto JsonRoot = OpenMetaDataJson();
    if (JsonRoot)
    {
        ToolPresetPath = JsonRoot->GetStringField("LastUsedPreset");
        MetaDataLoadExtension.ExecuteIfBound(JsonRoot);
        if (!ToolPresetPath.IsEmpty())
        {
            LoadStateFromJSON(ToolPresetPath, false);
        }
        else
        {
            auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
            auto SavedDir = ThisPlugin->GetBaseDir() + "/Saved";
            LoadStateFromJSON(SavedDir + "/" + DataName + "AutoSave" + ".json", false);
        }
    }
    else
        UE_LOG(LogTemp, Error, TEXT("Failed to load light control meta data."));

}