#include "ToolData.h"

#include "ItemHandle.h"

#include "ClassIconFinder.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"

#include "BaseLight.h"

#include "LightTreeHierarchy.h"

#include "Interfaces/IPluginManager.h"

UToolData::UToolData()
{
    SetFlags(GetFlags() | RF_Transactional);

    GenerateIcons();
    

}

UToolData::~UToolData()
{
    //AutoSave();
}

void UToolData::GenerateIcons()
{
    FLinearColor OffTint(0.2f, 0.2f, 0.2f, 0.5f);
    FLinearColor UndeterminedTint(0.8f, 0.8f, 0.0f, 0.5f);
    Icons.Emplace(SkyLightOn, *FClassIconFinder::FindThumbnailForClass(ASkyLight::StaticClass()));
    Icons.Emplace(SkyLightOff, Icons[SkyLightOn]);
    Icons[SkyLightOff].TintColor = OffTint;
    Icons.Emplace(SkyLightUndetermined, Icons[SkyLightOn]);
    Icons[SkyLightUndetermined].TintColor = UndeterminedTint;

    Icons.Emplace(DirectionalLightOn, *FClassIconFinder::FindThumbnailForClass(ADirectionalLight::StaticClass()));
    Icons.Emplace(DirectionalLightOff, Icons[DirectionalLightOn]);
    Icons[DirectionalLightOff].TintColor = OffTint;
    Icons.Emplace(DirectionalLightUndetermined, Icons[DirectionalLightOn]);
    Icons[DirectionalLightUndetermined].TintColor = UndeterminedTint;

    Icons.Emplace(SpotLightOn, *FClassIconFinder::FindThumbnailForClass(ASpotLight::StaticClass()));
    Icons.Emplace(SpotLightOff, Icons[SpotLightOn]);
    Icons[SpotLightOff].TintColor = OffTint;
    Icons.Emplace(SpotLightUndetermined, Icons[SpotLightOn]);
    Icons[SpotLightUndetermined].TintColor = UndeterminedTint;

    Icons.Emplace(PointLightOn, *FClassIconFinder::FindThumbnailForClass(APointLight::StaticClass()));
    Icons.Emplace(PointLightOff, Icons[PointLightOn]);
    Icons[PointLightOff].TintColor = OffTint;
    Icons.Emplace(PointLightUndetermined, Icons[PointLightOn]);
    Icons[PointLightUndetermined].TintColor = UndeterminedTint;

    Icons.Emplace(GeneralLightOn, Icons[PointLightOn]);
    Icons.Emplace(GeneralLightOff, Icons[PointLightOff]);
    Icons.Emplace(GeneralLightUndetermined, Icons[PointLightUndetermined]);

    Icons.Emplace(FolderClosed, *FEditorStyle::GetBrush("ContentBrowser.ListViewFolderIcon.Mask"));
    Icons.Emplace(FolderOpened, *FEditorStyle::GetBrush("ContentBrowser.ListViewFolderIcon.Base"));

    for (auto& Icon : Icons)
    {
        //Icon.Value.DrawAs = ESlateBrushDrawType::Box;
        Icon.Value.SetImageSize(FVector2D(24.0f));
    }
}

FCheckBoxStyle UToolData::MakeCheckboxStyleForType(uint8 IconType)
{
    FCheckBoxStyle CheckBoxStyle;
    CheckBoxStyle.CheckedImage = Icons[StaticCast<EIconType>(IconType * 3 + 1)];
    CheckBoxStyle.CheckedHoveredImage = Icons[StaticCast<EIconType>(IconType * 3 + 1)];
    CheckBoxStyle.CheckedPressedImage = Icons[StaticCast<EIconType>(IconType * 3 + 1)];

    CheckBoxStyle.UncheckedImage = Icons[StaticCast<EIconType>(IconType * 3 + 0)];
    CheckBoxStyle.UncheckedHoveredImage = Icons[StaticCast<EIconType>(IconType * 3 + 0)];
    CheckBoxStyle.UncheckedPressedImage = Icons[StaticCast<EIconType>(IconType * 3 + 0)];

    CheckBoxStyle.UndeterminedImage = Icons[StaticCast<EIconType>(IconType * 3 + 2)];
    CheckBoxStyle.UndeterminedHoveredImage = Icons[StaticCast<EIconType>(IconType * 3 + 2)];
    CheckBoxStyle.UndeterminedPressedImage = Icons[StaticCast<EIconType>(IconType * 3 + 2)];

    return CheckBoxStyle;
}

FSlateBrush& UToolData::GetIcon(EIconType Icon)
{
    return Icons[Icon];
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
    if (SelectedItems.Num())
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

        if (LoadingResult == UItemHandle::ELoadingResult::Success)
        {
            UE_LOG(LogTemp, Display, TEXT("Light control state loaded successfully"));
        }
        else
        {
            FString ErrorMessage = "";

            switch (LoadingResult)
            {
            case UItemHandle::ELoadingResult::LightNotFound:
                ErrorMessage = "At least one light could not be found. Please ensure all lights exist and haven't been renamed since the w.";
                break;
            case UItemHandle::ELoadingResult::EngineError:
                ErrorMessage = "There was an error with the engine. Please try loading again. If the error persists, restart the engine.";
                break;
            case UItemHandle::ELoadingResult::InvalidType:
                ErrorMessage = "The item type that was tried to be loaded was not valid. Please ensure that the item type in the .json file is between 0 and 4.";
                break;
            case UItemHandle::ELoadingResult::MultipleErrors:
                ErrorMessage = "Multiple errors occurred. See output log for more details.";
                break;
            }

            UE_LOG(LogTemp, Display, TEXT("Light control state could not load with following message: %s"), *ErrorMessage);

            FNotificationInfo NotificationInfo(FText::FromString(FString::Printf(TEXT("Light control tool state could not be loaded. Please check the output log."))));

            NotificationInfo.ExpireDuration = 300.0f;
            NotificationInfo.bUseSuccessFailIcons = false;

            FSlateNotificationManager::Get().AddNotification(NotificationInfo);
        }
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
        auto Content = ThisPlugin->GetContentDir();

        SaveStateToJson(Content + "/" + DataName + "AutoSave.json", false);
    }
    else
        SaveStateToJson(ToolPresetPath, false);

    SaveMetaData();
}

void UToolData::SaveMetaData()
{
    UE_LOG(LogTemp, Display, TEXT("Saving light control meta data."));
    auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
    auto Content = ThisPlugin->GetContentDir();

    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

    RootObject->SetStringField("LastUsedPreset", ToolPresetPath);
    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    FFileHelper::SaveStringToFile(Output, *(Content + "/" + DataName + "MetaData.json"));
}

void UToolData::LoadMetaData()
{
    auto ThisPlugin = IPluginManager::Get().FindPlugin("CradleLightControl");
    auto Content = ThisPlugin->GetContentDir();
    FString Input;
    if (FFileHelper::LoadFileToString(Input, *(Content + "/" + DataName + "MetaData.json")))
    {
        UE_LOG(LogTemp, Display, TEXT("Loading light control meta data."));
        TSharedPtr<FJsonObject> JsonRoot;
        TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Input);
        FJsonSerializer::Deserialize(JsonReader, JsonRoot);

        ToolPresetPath = JsonRoot->GetStringField("LastUsedPreset");
        if (!ToolPresetPath.IsEmpty())
        {
            LoadStateFromJSON(ToolPresetPath, false);
        }
        else
        {
            LoadStateFromJSON(Content + "/" + DataName + "AutoSave" + ".json", false);
        }
    }
    else
        UE_LOG(LogTemp, Error, TEXT("Failed to load light control meta data."));

}