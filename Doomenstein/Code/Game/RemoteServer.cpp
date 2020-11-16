#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/RemoteServer.hpp"
#include "Game/World.hpp"
#include "Game/Client.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MultiplayerGame.hpp"


//---------------------------------------------------------------------------------------------------------
RemoteServer::RemoteServer( std::string const& ipAddress, std::string const& portNum )
{
	g_theNetworkSystem->CreateTCPClient();

	uint16_t portAsNum = static_cast<uint16_t>( atoi( portNum.c_str() ) );
	g_theNetworkSystem->ConnectTCPClient( ipAddress, portAsNum, SocketMode::NONBLOCKING );

	TCPMessage createUDPMessage;

	m_udpListenPort = g_RNG->RollRandomIntInRange( 48000, 49000 );
	createUDPMessage.m_message = ToString( m_udpListenPort );

	createUDPMessage.m_header.m_id = MESSAGE_ID_UDP_SOCKET;
	createUDPMessage.m_header.m_size = createUDPMessage.m_message.size();

	g_theNetworkSystem->SendTCPMessage( createUDPMessage );
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::StartUp( GameType gameType )
{
	UNUSED( gameType );
	g_theGame = new MultiplayerGame();
	g_theGame->StartUp();
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ShutDown()
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
void RemoteServer::BeginFrame()
{
	ProcessTCPMessages();
	ProcessUDPMessages();

	SendInputData();
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::EndFrame()
{
	m_frameNum++;
	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		m_clients[clientIndex]->EndFrame();
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::Update()
{
	if( g_theGame != nullptr )
	{
		g_theGame->UpdateWorld();
	}

	for( int clientIndex = 0; clientIndex < m_clients.size(); ++clientIndex )
	{
		m_clients[clientIndex]->Update();
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::SendInputData()
{
	if( !g_theNetworkSystem->HasValidUDPSocket() )
		return;

	if( m_frameNum % 1 == 0 && !g_theConsole->IsOpen() )
	{
		InputState inputState = g_theInput->GetInputState();
		SendLargeUDPData( m_connectionIP, m_udpSendPort, &inputState, sizeof( InputState ), MESSAGE_ID_INPUT_DATA, m_frameNum );
		//g_theNetworkSystem->SendUDPMessage( inputMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::RequestConnectionData()
{
	UDPMessage connectionDataRequest;
	UDPMessageHeader& header = connectionDataRequest.m_header;
	
	memcpy( &header.m_fromAddress[0], &m_connectionIP[0], m_connectionIP.size() );
	header.m_frameNum = m_frameNum;
	header.m_id = MESSAGE_ID_CONNECTION_DATA;
	header.m_key = m_identifier;
	header.m_numMessages = 1;
	header.m_port = m_udpSendPort;
	header.m_seqNo = 0;
	header.m_size = 0;

	g_theNetworkSystem->SendUDPMessage( connectionDataRequest );
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::OpenUDPSocket( TCPMessage const& messageToProcess )
{
	m_identifier = messageToProcess.m_header.m_key;

	std::string udpSocketData = messageToProcess.m_message;
	Strings dataSplit = SplitStringOnDelimiter( udpSocketData, ':' );
	m_connectionIP = dataSplit[0];
	m_udpSendPort = atoi( dataSplit[1].c_str() );

	g_theNetworkSystem->OpenUDPPort( m_udpListenPort, m_udpSendPort );
	g_theConsole->PrintString( Rgba8::GREEN, "Open UDP Socket at %s listen on %i send on %i", m_connectionIP.c_str(), m_udpListenPort, m_udpSendPort );

	g_theNetworkSystem->DisconnectTCPClient();
	RequestConnectionData();
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessInputData( UDPMessage const& message )
{
}													   


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessEntityData( UDPMessage const& message )
{
	if( !IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket entityPacket( m_packets[MESSAGE_ID_ENTITY_DATA] );
	WorldData worldData;
	if( entityPacket.IsReadyToRead() )
	{
		memcpy( &worldData, &entityPacket.m_data[0], entityPacket.m_size );
		g_theGame->UpdateEntitiesFromWorldData( worldData );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessConnectionData( UDPMessage const& message )
{
	if( !IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket connectionPacket( m_packets[MESSAGE_ID_CONNECTION_DATA] );
	ConnectionData connectionData;
	if( connectionPacket.IsReadyToRead() )
	{
		memcpy( &connectionData, &connectionPacket.m_data[0], connectionPacket.m_size );
		g_theGame->SetCurrentMapByName( connectionData.m_currentMapByName );
		//g_theGame->SpawnEntitiesFromSpawnData( connectionData.m_entityData );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessCameraData( UDPMessage const& message )
{
	if( !IsValidMessage( message.m_header.m_key ) )
		return;

	UnpackUDPMessage( message );

	UDPPacket cameraPacket( m_packets[MESSAGE_ID_CAMERA_DATA] );
	CameraData cameraData;
	if( cameraPacket.IsReadyToRead() )
	{
		memcpy( &cameraData, &cameraPacket.m_data[0], cameraPacket.m_size );
		g_theGame->SetWorldCameraFromCameraData( cameraData );
	}
}
