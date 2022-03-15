// Copyright Epic Games, Inc. All Rights Reserved.

#include "CradleLightControlEditor.h"

#include "ClassIconFinder.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SpotLight.h"

#define LOCTEXT_NAMESPACE "FCradleLightControlEditorModule"

void FCradleLightControlEditorModule::StartupModule()
{
	GenerateIcons();

	CommandList = MakeShareable(new FUICommandList);

}

void FCradleLightControlEditorModule::ShutdownModule()
{
}

//void FCradleLightControlEditorModule::AddToolBarButton(FToolBarBuilder& ToolbarBuilder)
//{
//	FUIAction Action;
//	Action.ExecuteAction = FExecuteAction::CreateRaw(this, &FCradleLightControlEditorModule::OnToolBarButtonClicked);
//	ToolbarBuilder.AddToolBarButton(Action, NAME_None, FText::FromString("Cradle Light Control"));
//}

void FCradleLightControlEditorModule::GenerateIcons()
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
		Icon.Value.SetImageSize(FVector2D(24.0f));
	}
}

FSlateBrush& FCradleLightControlEditorModule::GetIcon(EIconType Icon)
{
	return Icons[Icon];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCradleLightControlEditorModule, CradleLightControl)