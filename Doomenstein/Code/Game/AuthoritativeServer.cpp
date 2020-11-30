#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/RemoteClient.hpp"
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
	case MULTI_PLAYER_GAME: 
	{
		g_theGame = new MultiplayerGame();
		m_identifier = g_RNG->RollRandomIntInRange(0, 200000000);
		g_theNetworkSystem->CreateTCPServer(SocketMode::NONBLOCKING);
		break;
	}
	default:
		break;
	}

	g_theGame->StartUp();

}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ShutDown()
{
	g_theNetworkSystem->CloseTCPServer();

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
	m_frameNum++;
	ProcessTCPMessages();
	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		m_clients[clientIndex]->SetFrameNum( m_frameNum );
		m_clients[clientIndex]->BeginFrame();
	}
	g_theGame->BeginFrame();
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::EndFrame()
{
	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		if( m_clients[clientIndex]->IsDisconnecting() )
		{
			delete m_clients[clientIndex];
			m_clients[clientIndex] = m_clients[m_clients.size() - 1];
			m_clients.pop_back();
		}
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
void AuthoritativeServer::OpenUDPSocket( TCPMessage const& message )
{
	int udpSendPort = g_RNG->RollRandomIntInRange( 48000, 49000 );
	int udpListenPort = udpSendPort + 1;
	std::string connectionData = g_theNetworkSystem->GetTCPSocketAddress();
	std::string connectionAddress = SplitStringOnDelimiter( connectionData, ':' )[0];
	UDPSocket* udpSocket = g_theNetworkSystem->OpenUDPPort( connectionAddress, udpListenPort, udpSendPort );

	TCPMessage udpConnectMessage;
	udpConnectMessage.m_message = Stringf( "%i-%i", udpSendPort, udpListenPort ); 

	udpConnectMessage.m_header.m_id = MESSAGE_ID_UDP_SOCKET;
	udpConnectMessage.m_header.m_key = m_identifier;
	udpConnectMessage.m_header.m_size = udpConnectMessage.m_message.size();

	g_theNetworkSystem->SendTCPMessage( udpConnectMessage );

	g_theConsole->PrintString( Rgba8::GREEN, "key: %i listen: %i send: %i", m_identifier, udpListenPort, udpSendPort );
	new RemoteClient( this, udpSocket );
}
