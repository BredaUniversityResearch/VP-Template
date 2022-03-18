#include "PhysicalObjectTrackingFilterSettings.h"

#if WITH_EDITOR
void UPhysicalObjectTrackingFilterSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName TargetSampleCountPropertyName = FName(TEXT("TargetSampleCount"));
	if (PropertyChangedEvent.MemberProperty != nullptr && PropertyChangedEvent.MemberProperty->GetFName() == TargetSampleCountPropertyName)
	{
		OnFilterSettingsChanged.Broadcast();
	}
}
#endif
