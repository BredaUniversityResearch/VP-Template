#include "PhysicalObjectTrackerSerialId.h"

#if WITH_EDITOR
void UPhysicalObjectTrackerSerialId::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty != nullptr && PropertyChangedEvent.MemberProperty->GetFName() == FName(TEXT("SerialId")))
	{
		OnSerialIdChanged.Broadcast();
	}
}
#endif