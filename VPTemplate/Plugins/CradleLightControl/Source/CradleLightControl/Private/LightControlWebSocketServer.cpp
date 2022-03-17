#include "LightControlWebSocketServer.h"

#include "IWebSocketNetworkingModule.h"

FLightControlWebSocketServer::FLightControlWebSocketServer() = default;

bool FLightControlWebSocketServer::Start(uint32 TargetPort)
{
	FWebSocketClientConnectedCallBack Callback;
	Callback.BindRaw(this, &FLightControlWebSocketServer::OnWebSocketClientConnected);

	m_Server = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();

	if (m_Server == nullptr || !m_Server->Init(TargetPort, Callback))
	{
		m_Server.Reset();
		return false;
	}

	return true;
}

void FLightControlWebSocketServer::Tick(float DeltaTime)
{
	m_Server->Tick();
}

bool FLightControlWebSocketServer::IsAllowedToTick() const
{
	return m_Server != nullptr;
}

void FLightControlWebSocketServer::OnWebSocketClientConnected(INetworkingWebSocket* NetworkingWebSocket)
{
	__debugbreak();
}
