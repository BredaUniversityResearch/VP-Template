#pragma once

#include "BMCCDataReceivedHandler.h"
#include "BMCCDispatcher.h"

struct FBMCCCommandPayloadBase;
struct FBMCCCommandIdentifier;
class FBMCCCommandMeta;

class BLACKMAGICCAMERACONTROL_API FBMCCService
	: public IModularFeature
	, public IBMCCDataReceivedHandler
	, public FTickableGameObject
{
	class Pimpl;
public:
	static FName GetModularFeatureName()
	{
		static const FName FeatureName = FName(TEXT("BlackmagicCameraControlService"));
		return FeatureName;
	}

	FBMCCService();
	virtual ~FBMCCService() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(DetectTrackerShakeTask, STATGROUP_ThreadPoolAsyncTasks); }
	virtual bool IsTickableInEditor() const override { return true; }

	virtual void OnDataReceived(BMCCDeviceHandle Source, const BMCCCommandHeader& Header, 
		const FBMCCCommandMeta& CommandMetaData, const TArrayView<uint8_t>& SerializedData) override;

	template<typename TCommandType>
	void BroadcastCommand(const TCommandType& Command);
	void BroadcastCommand(const FBMCCCommandIdentifier& Identifier, const FBMCCCommandPayloadBase& Command) const;

	void SubscribeMessageReceivedHandler(IBMCCCallbackHandler* a_CallbackHandler);
	void UnsubscribeMessageReceivedHandler(IBMCCCallbackHandler* a_CallbackHandler);

	UBMCCDispatcher* GetDefaultDispatcher() const;

private:
	TUniquePtr<Pimpl> m_Data;

	UBMCCDispatcher* DefaultDispatcher;
};

template <typename TCommandType>
void FBMCCService::BroadcastCommand(const TCommandType& Command)
{
	BroadcastCommand(TCommandType::Identifier, Command);
}
