#pragma once

#include "Slate.h"
#include "Widgets/SDMXPortSelector.h"

class SLightControlDMX : public SCompoundWidget
{
public:

SLATE_BEGIN_ARGS(SLightControlDMX) {}

SLATE_ARGUMENT(class SLightControlTool*, CoreToolPtr)

SLATE_END_ARGS()

void Construct(const FArguments& Args);

private:

    void OnPortSelected();

    ECheckBoxState UseDMXCheckBoxState() const;
    void UseDMXCheckBoxStateChanged(ECheckBoxState NewState);

    TOptional<int> StartingChannelGetValue() const;
    void StartingChannelValueChanged(int NewValue);

    void OnClassSelected(const UClass* Class);

    bool StartingChannelBoxIsEnabled() const;

    SLightControlTool* CoreToolPtr;

    TSharedPtr<SDMXPortSelector> PortSelector;
    TSharedPtr<IStructureDetailsView> DetailView;
    TSharedPtr<class FLightDMXNotifyHook> NotifyHook;
};
