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

void UPhysicalObjectTrackerSerialId::SetSerialId(const FString& InSerialId)
{
	SerialId = InSerialId;
	OnSerialIdChanged.Broadcast();
}

const FString& UPhysicalObjectTrackerSerialId::GetSerialId() const
{
	return SerialId;
}