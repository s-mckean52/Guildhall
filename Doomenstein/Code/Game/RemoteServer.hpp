#pragma once
#include "Game/Server.hpp"


class RemoteServer : public Server
{
	virtual void StartUp( GameType gameType )	override;
	virtual void ShutDown()						override;
	virtual void BeginFrame()					override;
	virtual void EndFrame()						override;
	virtual void Update()						override;
};