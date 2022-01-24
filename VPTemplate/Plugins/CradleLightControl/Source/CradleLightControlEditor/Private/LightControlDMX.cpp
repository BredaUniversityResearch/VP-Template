
#include "LightControlDMX.h"

#include "LightControlTool.h"
#include "IO/DMXOutputPort.h"

#include "ClassViewerModule.h"
#include "PropertyEditor/Public/PropertyEditing.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "IStructureDetailsView.h"
#include "LightTreeHierarchy.h"


void SLightControlDMX::Construct(const FArguments& Args)
{
    CoreToolPtr = Args._CoreToolPtr;

    //auto MasterLight = CoreToolPtr->GetMasterLight();
    //
    //if (MasterLight)
    //{
    //    SVerticalBox::FSlot* PortSelectorSlot;
    //    SVerticalBox::FSlot* DMXPropertiesSlot;

    //    FDetailsViewArgs DetailArgs;
    //    FStructureDetailsViewArgs StructArgs;
    //    TSharedPtr<FStructOnScope> StructScope = MakeShared<FStructOnScope>(FLightDMXProperties::StaticStruct(), reinterpret_cast<uint8*>(&MasterLight->DMXProperties));
    //    StructScope->SetPackage(MasterLight->GetOutermost());

    //    NotifyHook = MakeShared<FLightDMXNotifyHook>();
    //    NotifyHook->PropertiesRef = &MasterLight->DMXProperties;

    //    StructArgs.bShowClasses = true;
    //    StructArgs.bShowObjects = true;
    //    StructArgs.bShowAssets = true;
    //    StructArgs.bShowInterfaces = true;
    //    DetailArgs.bUpdatesFromSelection = true;
    //    DetailArgs.NotifyHook = NotifyHook.Get();
    //    DetailArgs.bAllowSearch = false;
    //    DetailView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(FName("PropertyEditor")).CreateStructureDetailView(DetailArgs, StructArgs, StructScope, FText::FromString("Nagibator bab"));



    //    ChildSlot
    //        [
    //            SNew(SVerticalBox)
    //            +SVerticalBox::Slot()
    //            .Expose(PortSelectorSlot)
    //            [
    //                SAssignNew(PortSelector, SDMXPortSelector)
    //                .Mode(EDMXPortSelectorMode::SelectFromAvailableInputsAndOutputs)
    //                .InitialSelection(MasterLight->DMXProperties.OutputPort ? MasterLight->DMXProperties.OutputPort->GetPortGuid() : FGuid())
    //                .OnPortSelected(this, &SLightControlDMX::OnPortSelected)
    //            ]
    //            +SVerticalBox::Slot()
    //            .Expose(DMXPropertiesSlot)
    //            [
    //                SNew(SVerticalBox)
    //                /*+ SVerticalBox::Slot()
    //                [
    //                    SNew(SHorizontalBox)
    //                    + SHorizontalBox::Slot()
    //                    [
    //                        SNew(STextBlock)
    //                        .Text(FText::FromString("Use DMX"))
    //                    ]
    //                    + SHorizontalBox::Slot()
    //                    [
    //                        SNew(SCheckBox)
    //                        .IsChecked(this, &SLightControlDMX::UseDMXCheckBoxState)
    //                        .OnCheckStateChanged(this, &SLightControlDMX::UseDMXCheckBoxStateChanged)

    //                    ]
    //                ]*/
    //                +SVerticalBox::Slot()                        
    //                [
    //                    SNew(SHorizontalBox)
    //                    + SHorizontalBox::Slot()
    //                    [
    //                        SNew(SBox)
    //                        .HeightOverride(120.0f)
    //                        [
    //                            DetailView->GetWidget().ToSharedRef()
    //                        ]
    //                    ]
    //                ]
    //                /*
    //                +SVerticalBox::Slot()
    //                [
    //                    SNew(SHorizontalBox)
    //                    +SHorizontalBox::Slot()
    //                    [
    //                        SNew(STextBlock)
    //                        .Text(FText::FromString("Starting channel"))
    //                    ]
    //                    +SHorizontalBox::Slot()
    //                    [
    //                        SNew(SNumericEntryBox<int>)
    //                        .OnValueChanged(this, &SLightControlDMX::StartingChannelValueChanged)
    //                        .Value(this, &SLightControlDMX::StartingChannelGetValue)                            
    //                        .IsEnabled(this, &SLightControlDMX::StartingChannelBoxIsEnabled)
    //                    ]
    //                ]*/
    //            ]                
    //        ];
    //           

    //    PortSelectorSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    //    DMXPropertiesSlot->SizeParam.SizeRule = FSizeParam::SizeRule_Auto;
    //}
}

void SLightControlDMX::OnPortSelected()
{
    //auto OutputPort = PortSelector->GetSelectedOutputPort();
    //if (OutputPort)
    //{
    //    TMap<int32, uint8> Channels;
    //    OutputPort->SendDMX(1, Channels);
    //    auto MasterLight = CoreToolPtr->GetMasterLight();
    //    MasterLight->DMXProperties.OutputPort = OutputPort;
    //    if (MasterLight)
    //    {
    //        //MasterLight->UpdateDMX();
    //    }
    //}
}
TOptional<int> SLightControlDMX::StartingChannelGetValue() const
{
    /*auto Master = CoreToolPtr->GetMasterLight();

    if (Master && Master->DMXProperties.DataConverter)
    {
        return Master->DMXProperties.DataConverter->StartingChannel;
    }*/
    return 0;
}

void SLightControlDMX::StartingChannelValueChanged(int NewValue)
{
    //auto Master = CoreToolPtr->GetMasterLight();

    //if (Master)
    //{
    //    Master->DMXProperties.DataConverter->StartingChannel = NewValue;
    //    //Master->UpdateDMX();
    //}
}

void SLightControlDMX::OnClassSelected(const UClass* Class)
{

}

ECheckBoxState SLightControlDMX::UseDMXCheckBoxState() const
{
    /*auto Master = CoreToolPtr->GetMasterLight();

    if (Master && Master->DMXProperties.DataConverter)
    {
        return Master->DMXProperties.bUseDmx ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;        
    }*/
    return ECheckBoxState::Undetermined;
}

void SLightControlDMX::UseDMXCheckBoxStateChanged(ECheckBoxState NewState)
{
    /*auto Master = CoreToolPtr->GetMasterLight();

    if (Master && Master->DMXProperties.DataConverter)
    {
        Master->DMXProperties.bUseDmx = NewState == ECheckBoxState::Checked;
    }*/
}

bool SLightControlDMX::StartingChannelBoxIsEnabled() const
{
    /*auto Master = CoreToolPtr->GetMasterLight();

    return Master && Master->DMXProperties.DataConverter;*/
    return false;
}
