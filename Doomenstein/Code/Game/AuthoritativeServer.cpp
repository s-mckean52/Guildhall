#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DevConsole.hpp"
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

	g_theNetworkSystem->CreateTCPServer( SocketMode::NONBLOCKING );
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
	ProcessTCPMessages();
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


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessTCPMessages()
{
	for( ;; )
	{
		TCPMessage* newMessage = g_theNetworkSystem->GetTCPMessage();
		if( newMessage == nullptr )
			break;

		ProcessTCPMessage( newMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessTCPMessage( TCPMessage* message )
{
	TCPMessageHeader header = message->m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_UDP_REQUEST: { OpenUDPSocket( message->m_message ); } break;

	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::OpenUDPSocket( std::string const& udpSocketData )
{
	uint16_t randomKey = static_cast<uint16_t>( g_RNG->RollRandomIntInRange( 0, 2000000 ) );
	uint16_t sendToPort = static_cast<uint16_t>( atoi( udpSocketData.c_str() ) );
	uint16_t listenPort = sendToPort + 1;
	g_theNetworkSystem->OpenUDPPort( listenPort, sendToPort );

	TCPMessage udpConnectMessage;
	udpConnectMessage.m_message = Stringf( "%s:%i", "127.0.0.1", listenPort ); 

	udpConnectMessage.m_header.m_id = MESSAGE_ID_UDP_SOCKET;
	udpConnectMessage.m_header.m_key = randomKey;
	udpConnectMessage.m_header.m_size = udpConnectMessage.m_message.size();

	g_theNetworkSystem->SendTCPMessage( udpConnectMessage );

	g_theConsole->PrintString( Rgba8::GREEN, "key: %i listen: %i send: %i", randomKey, listenPort, sendToPort );
}
