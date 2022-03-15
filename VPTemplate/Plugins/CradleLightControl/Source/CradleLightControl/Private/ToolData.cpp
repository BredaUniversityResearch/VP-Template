#include "ToolData.h"

#include "Engine/DirectionalLight.h"

#include "BaseLight.h"

#include "LightControlLoadingResult.h"
#include "Json.h"
#include "CradleLightControl.h"

//#include "LightTreeHierarchy.h"

#include "Interfaces/IPluginManager.h"

UToolData::UToolData()
	: LightIdCounter(0)
{
    SetFlags(GetFlags() | RF_Transactional);


}

UToolData::~UToolData()
{
    //AutoSave();
}

UBaseLight* UToolData::GetLightByName(FString Name)
{
	for (auto& Light : Lights)
	{
		if (Light->Name == Name)
		{
            return Light;
		}
	}

	if (GEngine)
	{
        GEngine->AddOnScreenDebugMessage(1999, 0.5f, FColor::Cyan, FString::Printf(TEXT("Could not find item with name \"%s\""), *Name));
	}

    return nullptr;
}

#if WITH_EDITOR
void UToolData::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
    OnTransacted.ExecuteIfBound(TransactionEvent);
}
#endif
void UToolData::BeginTransaction()
{
    Modify();
}


void UToolData::ClearAllData()
{
    Lights.Empty();
}

UBaseLight* UToolData::AddItem(bool bAssignId)
{
    auto Item = NewObject<UBaseLight>(this, ItemClass);
    if (bAssignId)
		Item->Id = LightIdCounter++;
    Item->OwningToolData = this;
    Lights.Add(Item);


    return Item;
}

TSharedPtr<FJsonObject> UToolData::SaveStateToJson(FString Path, bool bUpdatePresetPath)
{
    TArray<TSharedPtr<FJsonValue>> TreeItemsJSON;

    for (auto Light : Lights)
    {
        auto JsonObject = MakeShared<FJsonObject>();
        TSharedPtr<FJsonValue> JsonValue = MakeShared<FJsonValueObject>(JsonObject);

        JsonObject->SetObjectField("Light", Light->SaveAsJson());


        TreeItemsJSON.Emplace(JsonValue);
    }
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

    RootObject->SetArrayField("Lights", TreeItemsJSON);

    if (bUpdatePresetPath)
        ToolPresetPath = Path;

    return RootObject;
}

TSharedPtr<FJsonObject> UToolData::LoadStateFromJSON(FString Path, bool bUpdatePresetPath)
{
    bCurrentlyLoading = true;

    FString Input;
    //GEngine->AddOnScreenDebugMessage(328 + DataName.Len(), 60.0f, FColor::Magenta,
    //    FString::Printf(TEXT("Trying to load data from %s"), *Path));
    if (FFileHelper::LoadFileToString(Input, *Path))
    {
        GEngine->AddOnScreenDebugMessage(228 + DataName.Len(), 60.0f, FColor::Magenta,
            FString::Printf(TEXT("Successfully loaded data for %s"), *DataName));
        if (bUpdatePresetPath)
            ToolPresetPath = Path;
        UE_LOG(LogTemp, Display, TEXT("Beginning light control tool state loading from %s"), *Path);
        TSharedPtr<FJsonObject> JsonRoot;
        TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Input);
        FJsonSerializer::Deserialize(JsonReader, JsonRoot);

        auto LoadingResult = ELightControlLoadingResult::Success;
        for (auto Light : JsonRoot->GetArrayField("Lights"))
        {
            const TSharedPtr<FJsonObject>* TreeElementObjectPtr;
            auto Success = Light->TryGetObject(TreeElementObjectPtr);
            auto TreeElementObject = (*TreeElementObjectPtr)->GetObjectField("Light");
            check(Success);
            int Type = TreeElementObject->GetNumberField("Type");
            auto Item = AddItem(false); // Do not assign ID since we are going to load it from the file
            auto Res = Item->LoadFromJson(TreeElementObject);

        	LightIdCounter = FMath::Max(Item->Id, LightIdCounter); 
            if (Res != ELightControlLoadingResult::Success)
            {
                if (LoadingResult == ELightControlLoadingResult::Success)
                {
                    LoadingResult = Res;
                }
                else LoadingResult = ELightControlLoadingResult::MultipleErrors;
            }

        }
        TreeStructureChangedDelegate.ExecuteIfBound();

        OnToolDataLoaded.ExecuteIfBound(LoadingResult);
        bCurrentlyLoading = false;

        return JsonRoot;
     
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Could not open file %s"), *Path);
        ToolPresetPath = "";
    }

    //GEngine->AddOnScreenDebugMessage(228 + DataName.Len(), 60.0f, FColor::Magenta,
    //    FString::Printf(TEXT("%lu root elements loaded for %s"), RootItems.Num(), *DataName));
    bCurrentlyLoading = false;
    return nullptr;
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
    //GEngine->AddOnScreenDebugMessage(128 + DataName.Len(), 60.0f, FColor::Magenta,
    //    FString::Printf(TEXT("Loading metadata for %s"), *DataName));
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
            //GEngine->AddOnScreenDebugMessage(238 + DataName.Len(), 60.0f, FColor::Magenta,
            //    FString::Printf(TEXT("%s data being loaded from given path"), *DataName));
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