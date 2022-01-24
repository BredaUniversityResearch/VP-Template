#pragma once
#include "BlackmagicCameraControl.h"
#include "WinRT.h"

template<typename TClassType, typename TResultType>
struct AsyncWrapper
{
	using TargetFunctionConst = void(TClassType::*)(const TResultType&) const;
	using TargetFunction = void(TClassType::*)(const TResultType&);
	const TClassType* m_TargetConst;
	TargetFunctionConst m_FnTargetConst;
	TClassType* m_Target;
	TargetFunction m_FnTarget;

	AsyncWrapper(const TClassType* Target, TargetFunctionConst FnTarget);
	AsyncWrapper(TClassType* Target, TargetFunction FnTarget);

	void operator() (winrt::Windows::Foundation::IAsyncOperation<TResultType> AsyncOpResult, winrt::Windows::Foundation::AsyncStatus status);
};

template <typename TClassType, typename TResultType>
AsyncWrapper<TClassType, TResultType>::AsyncWrapper(const TClassType* Target, TargetFunctionConst FnTarget)
	: m_TargetConst(Target)
	, m_FnTargetConst(FnTarget)
	, m_Target(nullptr)
	, m_FnTarget(nullptr)
{
}

template <typename TClassType, typename TResultType>
AsyncWrapper<TClassType, TResultType>::AsyncWrapper(TClassType* Target, TargetFunction FnTarget)
	: m_TargetConst(nullptr)
	, m_FnTargetConst(nullptr)
	, m_Target(Target)
	, m_FnTarget(FnTarget)
{
}

template <typename TClassType, typename TResultType>
void AsyncWrapper<TClassType, TResultType>::operator()(winrt::Windows::Foundation::IAsyncOperation<TResultType> AsyncOpResult, winrt::Windows::Foundation::AsyncStatus status)
{
	if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
	{
		if (m_FnTarget != nullptr)
		{
			(m_Target->*m_FnTarget)(AsyncOpResult.GetResults());
		}
		else
		{
			(m_Target->*m_FnTargetConst)(AsyncOpResult.GetResults());
		}
	}
	else
	{
		UE_LOG(LogBlackmagicCameraControl, Warning, TEXT("Async operation failed"));
	}
}

