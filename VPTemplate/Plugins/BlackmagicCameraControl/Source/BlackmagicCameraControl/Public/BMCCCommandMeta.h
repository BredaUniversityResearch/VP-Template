#pragma once
#include "BMCCCommandIdentifier.h"
#include "BMCCCallbackHandler.h"

class IBMCCCallbackHandler;
class FBMCCCommandMeta
{
	template<typename TCommandType>
	using DispatchFn = void(IBMCCCallbackHandler::*)(BMCCDeviceHandle, const TCommandType&);

	using DispatchWrapperFn = void(*)(IBMCCCallbackHandler* DispatchTarget, BMCCDeviceHandle Source, const TArrayView<uint8>& ArrayView);

public:
	const int PayloadSize;
	const FBMCCCommandIdentifier CommandIdentifier;
	const DispatchWrapperFn DispatchWrapper;

	static const FBMCCCommandMeta* FindMetaForIdentifier(const FBMCCCommandIdentifier& Identifier);

	void DeserializeAndDispatch(IBMCCCallbackHandler* DispatchTarget, BMCCDeviceHandle Source, const TArrayView<uint8>& ArrayView) const;

private:
	static const FBMCCCommandMeta m_AllMeta[];

	template<typename TCommandType, void(IBMCCCallbackHandler::*TDispatchFunction)(BMCCDeviceHandle, const TCommandType&), int PayloadSize = sizeof(TCommandType)>
	static FBMCCCommandMeta Create();

	constexpr FBMCCCommandMeta(FBMCCCommandIdentifier Identifier, int PayloadSize, DispatchWrapperFn DispatchWrapper)
		: PayloadSize(PayloadSize)
		, CommandIdentifier(Identifier)
		, DispatchWrapper(DispatchWrapper)
	{
	}
};
