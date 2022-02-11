
#include "DirectLightEditorNetwork.h"
#include "CradleLightControl.h"
#include "CradleLightControlEditor.h"

#include "EditorData.h"
#include "ItemHandle.h"
#include "LightTreeHierarchy.h"
#include "TreeItemWidget.h"

FDirectLightEditorNetwork::FDirectLightEditorNetwork(UEditorData* InVirtualLightEditorData,
	UEditorData* InDMXLightEditorData)
		: ILightEditorNetwork(InVirtualLightEditorData, InDMXLightEditorData)
{
    VirtualLightEditorData = InVirtualLightEditorData;
    DMXLightEditorData = InDMXLightEditorData;
}

void FDirectLightEditorNetwork::SendLightPropertyChangeEvent(EDataSet TargetDataSet, TArray<UBaseLight*>& AffectedLights, EProperty Property, float Value)
{

    FCradleLightControlModule::GetRuntimeNetworkInterface().OnPropertiesChanged(TargetDataSet, AffectedLights, Property, Value);
}

void FDirectLightEditorNetwork::OnNewVirtualLightSpawned(UBaseLight* NewLight)
{
    auto NewHandle = VirtualLightEditorData->AddItem();
    NewHandle->Name = NewLight->Name;
    NewHandle->Item = NewLight;
    NewHandle->LightId = NewLight->Id;
    NewHandle->Parent = nullptr;
    
    //DataUpdateDelegate.ExecuteIfBound(NewLight);
    auto TreeWidget = FCradleLightControlEditorModule::Get().VirtualLightControl->GetTreeWidget().Pin();
    TreeWidget->GenerateWidgetForItem(NewHandle);
    TreeWidget->GetWidgetForItem(NewHandle)->CheckNameAgainstSearchString(TreeWidget->SearchString);

    VirtualLightEditorData->RootItems.Add(NewHandle);
    VirtualLightEditorData->ListOfLightItems.Add(NewHandle);

    TreeWidget->Tree->RequestTreeRefresh();
}

void FDirectLightEditorNetwork::RemoveInvalidVirtualLightsFromEditor(TArray<UBaseLight*> LightsToRemove)
{
    
    for (auto Light : LightsToRemove)
    {
        auto Ptr = VirtualLightEditorData->ListOfLightItems.FindByPredicate([Light](UItemHandle* Handle)
            {
                return Light->Id == Handle->LightId;
            });
        if (Ptr) 
        {
            auto LightToRemove = *Ptr;
            VirtualLightEditorData->RootItems.Remove(LightToRemove);
            VirtualLightEditorData->ListOfTreeItems.Remove(LightToRemove);
            VirtualLightEditorData->ListOfLightItems.Remove(LightToRemove);
        }
    }

    FCradleLightControlEditorModule::Get().VirtualLightControl->GetTreeWidget().Pin()->Tree->RequestTreeRefresh();
}
