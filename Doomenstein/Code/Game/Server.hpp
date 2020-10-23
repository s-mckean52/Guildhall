#pragma once
#include "Game/GameCommon.hpp"
#include <vector>

class Game;
class Client;

class Server
{
public:
	Server();
	virtual ~Server();

	Game* GetGame() const;

	void AddClient( Client* playerClient );

	virtual void StartUp( GameType gameType )	= 0;
	virtual void ShutDown()						= 0;
	virtual void BeginFrame()					= 0;
	virtual void EndFrame()						= 0;
	virtual void Update()						= 0;

protected:
	std::vector<Client*> m_clients;
};