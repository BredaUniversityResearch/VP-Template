#pragma once

#include "IWebSocketServer.h"

class FLightControlWebSocketServer: public FTickableObjectBase
{
public:
	FLightControlWebSocketServer();

	bool Start(uint32 TargetPort);
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsAllowedToTick() const override;

	void OnWebSocketClientConnected(INetworkingWebSocket* NetworkingWebSocket);

private:
	TUniquePtr<IWebSocketServer> m_Server;

	FDelegateHandle m_TickHandle;
};

inline TStatId FLightControlWebSocketServer::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FLightControlWebSocketServer, STATGROUP_Tickables);
}
