#include "LightControlTool.h"

#include "Slate.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Chaos/AABB.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"

#include "ClassIconFinder.h"

#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"

#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"

#include "ToolData.h"
#include "ItemHandle.h"
#include "BaseLight.h"

#include "EditorData.h"
#include "LightEditorWidget.h"

#include "VirtualLight.h"



void SLightControlTool::Construct(const FArguments& Args, UToolData* InToolData)
{
    EditorData = NewObject<UEditorData>();
    EditorData->MetaDataSaveExtension = FMetaDataExtension::CreateRaw(this, &SLightControlTool::MetaDataSaveExtension);
    EditorData->MetaDataLoadExtension = FMetaDataExtension::CreateRaw(this, &SLightControlTool::MetaDataLoadExtension);

    SLightEditorWidget::Construct(Args, InToolData);


    // Build a database from what is in the level if there wasn't a file to recover from
    if (EditorData->RootItems.Num() == 0)
    {
        UpdateLightList();
    }

    DataAutoSaveTimer = RegisterActiveTimer(300.0f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
        {
            EditorData->AutoSave();

            return EActiveTimerReturnType::Continue;
        }));

    EditorData->AddToRoot();

    LightHierarchyWidget->DataVerificationDelegate.BindRaw(this, &SLightControlTool::VerifyTreeData);
    LightHierarchyWidget->DataUpdateDelegate.BindStatic(&SLightControlTool::UpdateItemData);

    FWorldDelegates::OnWorldCleanup.AddLambda([this](UWorld*, bool, bool)
        {
            EditorData->SaveMetaData();
            if (ActorSpawnedListenerHandle.IsValid())
            {
                GWorld->RemoveOnActorSpawnedHandler(ActorSpawnedListenerHandle);
                ActorSpawnedListenerHandle.Reset();
            }
        });

    FEditorDelegates::OnMapOpened.AddLambda([this](const FString&, bool)
    {
    
            ClearSelection();
			// Load the metadata for the current world
            EditorData->LoadMetaData();
            
			// If there is no concrete file associated with the current world, generate the data from scratch.
			// This way we avoid the tool using invalid data 
            if (EditorData->ToolPresetPath.IsEmpty())
            {
                UpdateLightList();
            }

            // TODO: This must be done even without the tool tab being spawned, so ideally on an event that signals the end of the initialization of the level
            if (!ActorSpawnedListenerHandle.IsValid() && GWorld)
            {
                auto ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateRaw(this, &SLightControlTool::ActorSpawnedCallback);
                ActorSpawnedListenerHandle = GWorld->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
            }
            else
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Could not set actor spawned callback");

            EditorData->bCurrentlyLoading = false;
        });
}

SLightControlTool::~SLightControlTool()
{
}

void SLightControlTool::PreDestroy()
{
    EditorData->AutoSave();
    if (LightHierarchyWidget)
        LightHierarchyWidget->PreDestroy();
    if (LightPropertyWidget)
        LightPropertyWidget->PreDestroy();

    FWorldDelegates::OnPostWorldInitialization.Remove(OnWorldChangedDelegateHandle);
    FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanupStartedDelegate);
    GWorld->RemoveOnActorSpawnedHandler(ActorSpawnedListenerHandle);
    UnRegisterActiveTimer(DataAutoSaveTimer.ToSharedRef());
}

void SLightControlTool::ActorSpawnedCallback(AActor* Actor)
{
    LightHierarchyWidget->OnActorSpawned(Actor);
}

void SLightControlTool::UpdateLightList()
{
    EditorData->ClearAllData();
    TArray<AActor*> Actors;
    // Fetch Point Lights

    UGameplayStatics::GetAllActorsOfClass(GWorld, ALight::StaticClass(), Actors);
    for (auto Light : Actors)
    {
        auto* NewItem = Cast<UVirtualLight>(EditorData->GetToolData()->AddItem());

        if (Cast<APointLight>(Light))
			NewItem->Type = ELightType::PointLight;
        else if (Cast<ADirectionalLight>(Light))
            NewItem->Type = ELightType::DirectionalLight;
        else if (Cast<ASpotLight>(Light))
            NewItem->Type = ELightType::SpotLight;
        else if (Cast<ASkyLight>(Light))
            NewItem->Type = ELightType::SkyLight;

        NewItem->Name = Light->GetName();
        NewItem->ActorPtr = Light;
        UpdateItemData(NewItem);

        // Need to create new handle here
        auto NewItemHandle = EditorData->AddItem();
        NewItemHandle->Name = NewItem->Name;
        NewItemHandle->Item = NewItem;
        EditorData->RootItems.Add(NewItemHandle);
        EditorData->ListOfLightItems.Add(NewItemHandle);
        LightHierarchyWidget->GenerateWidgetForItem(NewItemHandle);
    }
    
    if (LightHierarchyWidget)
		LightHierarchyWidget->Tree->RequestTreeRefresh();
}

void SLightControlTool::UpdateItemData(UBaseLight* BaseLight)
{
    check(BaseLight);
    auto VirtualLight = Cast<UVirtualLight>(BaseLight);
    FLinearColor RGB;

    BaseLight->Intensity = 0.0f;
    BaseLight->Saturation = 0.0f;
    BaseLight->Temperature = 0.0f;

    if (BaseLight->Type == ELightType::SkyLight)
    {
        RGB = VirtualLight->SkyLight->GetLightComponent()->GetLightColor();
        BaseLight->bIsEnabled = VirtualLight->SkyLight->GetLightComponent()->IsVisible();
    }
    else
    {
        ALight* LightPtr = Cast<ALight>(VirtualLight->ActorPtr);
        RGB = LightPtr->GetLightColor();
        BaseLight->bIsEnabled = LightPtr->GetLightComponent()->IsVisible();
    }
    auto HSV = RGB.LinearRGBToHSV();
    BaseLight->Saturation = HSV.G;

    // If Saturation is 0, the color is white. The RGB => HSV conversion calculates the Hue to be 0 in that case, even if it's not supposed to be.
    // Do this to preserve the Hue previously used rather than it getting reset to 0.
    if (BaseLight->Saturation != 0.0f)
        BaseLight->Hue = HSV.R;

    if (BaseLight->Type == ELightType::PointLight)
    {
        auto Comp = VirtualLight->PointLight->PointLightComponent;
        BaseLight->Intensity = Comp->Intensity;
    }
    else if (BaseLight->Type == ELightType::SpotLight)
    {
        auto Comp = VirtualLight->SpotLight->SpotLightComponent;
        BaseLight->Intensity = Comp->Intensity;
    }

    if (BaseLight->Type != ELightType::SkyLight)
    {
        auto LightPtr = Cast<ALight>(VirtualLight->ActorPtr);
        auto LightComp = LightPtr->GetLightComponent();
        BaseLight->bUseTemperature = LightComp->bUseTemperature;
        BaseLight->Temperature = LightComp->Temperature;

        VirtualLight->bCastShadows = LightComp->CastShadows;
    }
    else
    {
        VirtualLight->bCastShadows = VirtualLight->SkyLight->GetLightComponent()->CastShadows;
    }

    auto CurrentFwd = FQuat::MakeFromEuler(FVector(0.0f, BaseLight->Vertical, BaseLight->Horizontal)).GetForwardVector();
    auto ActorQuat = VirtualLight->ActorPtr->GetTransform().GetRotation().GetNormalized();
    auto ActorFwd = ActorQuat.GetForwardVector();

    if (CurrentFwd.Equals(ActorFwd))
    {
        auto Euler = ActorQuat.Euler();
        BaseLight->Horizontal = Euler.Z;
        BaseLight->Vertical = Euler.Y;
    }


    if (BaseLight->Type == ELightType::SpotLight)
    {
        BaseLight->InnerAngle = VirtualLight->SpotLight->SpotLightComponent->InnerConeAngle;
        BaseLight->OuterAngle = VirtualLight->SpotLight->SpotLightComponent->OuterConeAngle;
    }

}

void SLightControlTool::VerifyTreeData()
{
    if (EditorData->bCurrentlyLoading)
        return;
    
    GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, "Cleaning invalid lights");
    TArray<UItemHandle*> ToRemove;
    for (auto ItemHandle : EditorData->ListOfLightItems)
    {
        auto Item = Cast<UVirtualLight>(ItemHandle->Item);
        if (!Item->ActorPtr || !IsValid(Item->SkyLight))
        {
            if (ItemHandle->Parent)
                ItemHandle->Parent->Children.Remove(ItemHandle);
            else
                EditorData->RootItems.Remove(ItemHandle);


            ToRemove.Add(ItemHandle);
        }
        else
        {
            UpdateItemData(ItemHandle->Item);
        }
    }

    for (auto ItemHandle : ToRemove)
    {
        EditorData->GetToolData()->Lights.Remove(ItemHandle->Item);
        EditorData->RootItems.Remove(ItemHandle);
        EditorData->ListOfTreeItems.Remove(ItemHandle);
        EditorData->ListOfLightItems.Remove(ItemHandle);
    }

    if (ToRemove.Num())
    {
        LightHierarchyWidget->Tree->RequestTreeRefresh();
    }
}


void SLightControlTool::MetaDataSaveExtension(TSharedPtr<FJsonObject> RootJson)
{
	if (GWorld)
	{
        auto OpenedJson = EditorData->OpenMetaDataJson();
        if (OpenedJson)
            *RootJson = *OpenedJson; // Replace whatever was done by the default saving with the current file state
        else
            *RootJson = FJsonObject();

        auto MapName = GWorld->GetMapName();

        RootJson->SetStringField(MapName, EditorData->ToolPresetPath);
		
	}


}

void SLightControlTool::MetaDataLoadExtension(TSharedPtr<FJsonObject> RootJson)
{
	if (GWorld)
	{
        auto MapName = GWorld->GetMapName();

        if (RootJson->HasField(MapName))
        {
            EditorData->ToolPresetPath = RootJson->GetStringField(MapName);	        
        }
	}
}

void SLightControlTool::UpdateExtraLightDetailBox()
{
    if (EditorData->IsAMasterLightSelected())
    {
        if (EditorData->MultipleLightsInSelection())
        {
            ExtraLightDetailBox->SetContent(GroupControls());
        }
        else
        {
            ExtraLightDetailBox->SetContent(LightTransformViewer());
        }
    }
    else
        ExtraLightDetailBox->SetContent(SNew(SBox));
}

TSharedRef<SBox> SLightControlTool::LightTransformViewer()
{
    SHorizontalBox::FSlot* ButtonsSlot;

    TSharedPtr<SBox> Box;

    SAssignNew(Box, SBox)
    [
        SNew(SBorder)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .HAlign(HAlign_Fill)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Parent Object"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemParentName)
                    ]
                ]
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Position"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemPosition)
                    ]
                ]
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Rotation"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemRotation)
                    ]
                ]
                +SVerticalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Padding(5.0f, 3.0f)
                [
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Scale"))
                    ]
                    +SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    [
                        SNew(STextBlock)
                        .Text(this, &SLightControlTool::GetItemScale)
                    ]
                ]
            ]
            +SHorizontalBox::Slot()
            .Expose(ButtonsSlot)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Select Scene Object"))
                    .OnClicked(this, &SLightControlTool::SelectItemInScene)
                ]
                + SVerticalBox::Slot()
                .Padding(5.0f)
                [
                    SNew(SButton)
                    .Text(FText::FromString("Select Parent Object"))
                    .IsEnabled(this, &SLightControlTool::SelectItemParentButtonEnable)
                    .OnClicked(this, &SLightControlTool::SelectItemParent)
                ]
            ]
        ]
    ];

    ButtonsSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    return Box.ToSharedRef();
}

FReply SLightControlTool::SelectItemInScene()
{
    if (EditorData->IsAMasterLightSelected())
    {
        GEditor->SelectNone(true, true);
        GEditor->SelectActor(Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr, true, true, false, true);
    }

    return FReply::Handled();
}

FReply SLightControlTool::SelectItemParent()
{
    GEditor->SelectNone(true, true);
    GEditor->SelectActor(Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor(), true, true, false, true);

    return FReply::Handled();
}

bool SLightControlTool::SelectItemParentButtonEnable() const
{
    return EditorData->IsAMasterLightSelected() && Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor();
}

FText SLightControlTool::GetItemParentName() const
{
    if (EditorData->IsAMasterLightSelected() && Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor())
    {
        return FText::FromString(Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetAttachParentActor()->GetName());
    }
    return FText::FromString("None");
}

FText SLightControlTool::GetItemPosition() const
{
    if (EditorData->IsAMasterLightSelected())
    {
        return FText::FromString(Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetActorLocation().ToString());
    }
    return FText::FromString("");
}

FText SLightControlTool::GetItemRotation() const
{
    if (EditorData->IsAMasterLightSelected())
    {
        return FText::FromString(Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetActorRotation().ToString());
    }
    return FText::FromString("");
}

FText SLightControlTool::GetItemScale() const
{
    if (EditorData->IsAMasterLightSelected())
    {
        return FText::FromString(Cast<UVirtualLight>(EditorData->SelectionMasterLight->Item)->ActorPtr->GetActorScale().ToString());
    }
    return FText::FromString("");
}

TSharedRef<SBox> SLightControlTool::GroupControls()
{
    TSharedPtr<SBox> Box;

    
    SVerticalBox::FSlot* MasterLightSlot;
    SAssignNew(Box, SBox)
    [
        SNew(SBorder)
        .Padding(FMargin(5.0f, 5.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .Expose(MasterLightSlot)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Master Light"))
                ]
                + SHorizontalBox::Slot()
                [
                    SNew(SComboBox<UItemHandle*>)
                    .OptionsSource(&EditorData->LightsUnderSelection)
                    .OnGenerateWidget(this, &SLightControlTool::GroupControlDropDownLabel)
                    .OnSelectionChanged(this, &SLightControlTool::GroupControlDropDownSelection)
                    .InitiallySelectedItem(EditorData->SelectionMasterLight)[
                        SNew(STextBlock).Text(this, &SLightControlTool::GroupControlDropDownDefaultLabel)
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Affected lights"))
                ]
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Text(this, &SLightControlTool::GroupControlLightList)
                    .AutoWrapText(true)
                ]
            ]
        ]
    ];

    MasterLightSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;


    return Box.ToSharedRef();
}

TSharedRef<SWidget> SLightControlTool::GroupControlDropDownLabel(UItemHandle* ItemHandle)
{
    if (!ItemHandle->Item)
    {
        return SNew(SBox);
    }
    return SNew(STextBlock).Text(FText::FromString(ItemHandle->Name));
}

void SLightControlTool::GroupControlDropDownSelection(UItemHandle* Item, ESelectInfo::Type SelectInfoType)
{
    EditorData->SelectionMasterLight = Item;
    LightSpecificPropertiesWidget->UpdateToolState();
}

FText SLightControlTool::GroupControlDropDownDefaultLabel() const
{
    if (EditorData->SelectionMasterLight)
    {
        return FText::FromString(EditorData->SelectionMasterLight->Name);
    }
    return FText::FromString("");
}

FText SLightControlTool::GroupControlLightList() const
{
    FString LightList = EditorData->LightsUnderSelection[0]->Name;

    for (size_t i = 1; i < EditorData->LightsUnderSelection.Num(); i++)
    {
        LightList += ", ";
        LightList += EditorData->LightsUnderSelection[i]->Name;
    }

    return FText::FromString(LightList);
}
