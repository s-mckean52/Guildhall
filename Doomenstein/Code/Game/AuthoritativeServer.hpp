#pragma once
#include "Game/Server.hpp"


class AuthoritativeServer : public Server
{
public:
	void StartUp( GameType gameType )	override;
	void ShutDown()						override;
	void BeginFrame()					override;
	void EndFrame()						override;
	void Update()						override;
};