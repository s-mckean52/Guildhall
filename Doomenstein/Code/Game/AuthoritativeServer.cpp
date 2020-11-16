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
	ProcessUDPMessages();
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
		SendWorldDataToClient( m_clients[clientIndex] );
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
	std::string udpSocketData = message.m_message;

	m_udpSendPort = static_cast<uint16_t>( atoi( udpSocketData.c_str() ) );
	m_udpListenPort = m_udpSendPort + 1;
	g_theNetworkSystem->OpenUDPPort( m_udpListenPort, m_udpSendPort );

	TCPMessage udpConnectMessage;
	udpConnectMessage.m_message = Stringf( "%s:%i", "127.0.0.1", m_udpListenPort ); 

	udpConnectMessage.m_header.m_id = MESSAGE_ID_UDP_SOCKET;
	udpConnectMessage.m_header.m_key = m_identifier;
	udpConnectMessage.m_header.m_size = udpConnectMessage.m_message.size();

	g_theNetworkSystem->SendTCPMessage( udpConnectMessage );

	g_theConsole->PrintString( Rgba8::GREEN, "key: %i listen: %i send: %i", m_identifier, m_udpListenPort, m_udpSendPort );
	new RemoteClient( this );
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessInputData( UDPMessage const& message )
{
	if( !IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket inputPacket( m_packets[MESSAGE_ID_INPUT_DATA] );
	InputState newInputState;
	if( inputPacket.IsReadyToRead() )
	{
		memcpy( &newInputState, &inputPacket.m_data[0], sizeof( InputState ) );
		static_cast<RemoteClient*>( m_clients[1] )->SetInputFromInputState( newInputState );
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessEntityData( UDPMessage const& message )
{
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessConnectionData( UDPMessage const& message )
{
	SendSetupMessage( nullptr );
	SendWorldDataToClient( nullptr );
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessCameraData( UDPMessage const& message )
{
	if( !IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket cameraPacket( m_packets[MESSAGE_ID_CAMERA_DATA] );
	CameraData cameraData;
	if( cameraPacket.IsReadyToRead() )
	{
		memcpy( &cameraData, &cameraPacket.m_data[0], sizeof( CameraData ) );
		static_cast<RemoteClient*>( m_clients[1] )->SetCameraFromCameraData( cameraData );
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::SendWorldDataToClient( Client* client )
{
	if( !g_theNetworkSystem->HasValidUDPSocket() )
		return;
	
	if( m_frameNum % 6 == 0 )
	{
		WorldData entityData = g_theGame->GetWorldData();
		SendLargeUDPData( m_connectionIP, m_udpSendPort, &entityData, sizeof( entityData ), MESSAGE_ID_ENTITY_DATA, m_frameNum );
	}
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::SendSetupMessage( Client* client )
{
	if( !g_theNetworkSystem->HasValidUDPSocket() )
		return;
	
	ConnectionData setupData = g_theGame->GetConnectionData();
	SendLargeUDPData( m_connectionIP, m_udpSendPort, &setupData, sizeof( setupData ), MESSAGE_ID_CONNECTION_DATA, m_frameNum );
}


//---------------------------------------------------------------------------------------------------------
void AuthoritativeServer::SendCameraData( Client* client )
{
	if( !g_theNetworkSystem->HasValidUDPSocket() )
		return;
	
	CameraData cameraDataToSend = static_cast<RemoteClient*>( client )->GetCameraData();
	SendLargeUDPData( m_connectionIP, m_udpSendPort, &cameraDataToSend, sizeof( CameraData ), MESSAGE_ID_CAMERA_DATA, m_frameNum );
}
