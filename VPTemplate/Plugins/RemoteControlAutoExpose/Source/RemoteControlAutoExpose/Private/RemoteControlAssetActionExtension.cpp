#include "RemoteControlAssetActionExtension.h"

#include "RemoteControlEntity.h"
#include "RemoteControlPreset.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/Light.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "RemoteControlAutoExpose"

TSharedRef<FExtender> FRemoteControlAssetActionExtension::CreateMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddMenuExtension(
		"CommonAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FRemoteControlAssetActionExtension::MenuExtenderImpl, SelectedAssets)
	);
	return Extender;
}

void FRemoteControlAssetActionExtension::MenuExtenderImpl(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	if (SelectedAssets.Num() == 1 && SelectedAssets[0].GetClass() == URemoteControlPreset::StaticClass())
	{
		MenuBuilder.BeginSection("Remote Control", LOCTEXT("ASSET_CONTEXT", "Remote Control Auto Expose"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("SetFromScene", "Set From Current Scene"),
				LOCTEXT("SetFromSceneDesc", "Sets this preset up for use in the current scene."),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.ViewOptions", "LevelEditor.ViewOptions.Small"),
				FUIAction(FExecuteAction::CreateLambda([this, SelectedAssets]()
					{
						TArray<URemoteControlPreset*> typedSelectedAssets;
						for (const FAssetData& asset : SelectedAssets)
						{
							URemoteControlPreset* preset = Cast<URemoteControlPreset>(asset.GetAsset());
							if (preset != nullptr)
							{
								PopulateAssetFromCurrentScene(preset);
							}
						}
					})),
				NAME_None,
						EUserInterfaceActionType::Button);
		}
		MenuBuilder.EndSection();
	}
}

void FRemoteControlAssetActionExtension::PopulateAssetFromCurrentScene(URemoteControlPreset* TargetAsset) const
{
	TArray<AActor*> lightsInCurrentScene;
	UGameplayStatics::GetAllActorsOfClass(GWorld, ALight::StaticClass(), lightsInCurrentScene);
	for (AActor* actor : lightsInCurrentScene)
	{
		FProperty* propertyToExpose = ULightComponent::StaticClass()->FindPropertyByName(TEXT("LightColor"));

		UActorComponent* component = actor->GetComponentByClass(ULightComponent::StaticClass());

		TargetAsset->Bindings.Empty();
		TWeakPtr<FRemoteControlProperty> newExposedProperty = TargetAsset->ExposeProperty(component, FRCFieldPathInfo(propertyToExpose));
		if (newExposedProperty == nullptr)
		{
			__debugbreak();
		}
	}
}

#undef LOCTEXT_NAMESPACE
