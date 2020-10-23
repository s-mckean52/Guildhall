#include "Game/Game.hpp"
#include "Game/AuthoritativeServer.hpp"
#include "Game/PlayerClient.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::StartUp( GameType gameType )
{
	switch( gameType )
	{
	case SINGLE_PLAYER_GAME:	{ g_theGame = new SinglePlayerGame();	break; }
	case MULTI_PLAYER_GAME:		{ g_theGame = new MultiplayerGame();	break; }
	default:					{ break; }
	}

	g_theGame->StartUp();
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ShutDown()
{
	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = nullptr;

	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		delete m_clients[clientIndex];
		m_clients[clientIndex] = nullptr;
	}
	m_clients.clear();
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::BeginFrame()
{
	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		m_clients[clientIndex]->BeginFrame();
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::EndFrame()
{
	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		m_clients[clientIndex]->EndFrame();
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::Update()
{
	g_theGame->Update();
	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		m_clients[clientIndex]->Update();
	}
}
