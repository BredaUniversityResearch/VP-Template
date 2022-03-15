// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum EIconType
{
	GeneralLightOff = 0,
	GeneralLightOn,
	GeneralLightUndetermined,
	SkyLightOff,
	SkyLightOn,
	SkyLightUndetermined,
	SpotLightOff,
	SpotLightOn,
	SpotLightUndetermined,
	DirectionalLightOff,
	DirectionalLightOn,
	DirectionalLightUndetermined,
	PointLightOff,
	PointLightOn,
	PointLightUndetermined,
	FolderClosed,
	FolderOpened
};

class FCradleLightControlEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	//void AddToolBarButton(FToolBarBuilder& ToolbarBuilder);

	void GenerateIcons();
	FSlateBrush& GetIcon(EIconType Icon);

	TSharedPtr<FUICommandList> CommandList;
	
	TMap<TEnumAsByte<EIconType>, FSlateBrush> Icons;
private:

};
