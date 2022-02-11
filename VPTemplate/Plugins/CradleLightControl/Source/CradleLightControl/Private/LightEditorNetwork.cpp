#include "LightEditorNetwork.h"


ILightEditorNetwork::ILightEditorNetwork(UEditorData* InVirtualLightEditorData, UEditorData* InDMXLightEditorData)
	: VirtualLightEditorData(InVirtualLightEditorData)
	, DMXLightEditorData(InDMXLightEditorData)
{
}
