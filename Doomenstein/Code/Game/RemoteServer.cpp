#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Network/UDPSocket.hpp"
#include "Game/RemoteServer.hpp"
#include "Game/World.hpp"
#include "Game/Client.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Game/App.hpp"


//---------------------------------------------------------------------------------------------------------
RemoteServer::RemoteServer( std::string const& ipAddress, std::string const& portNum )
{
	m_connectionIP = ipAddress;

	g_theNetworkSystem->CreateTCPClient();

	uint16_t portAsNum = static_cast<uint16_t>( atoi( portNum.c_str() ) );
	g_theNetworkSystem->ConnectTCPClient( ipAddress, portAsNum, SocketMode::NONBLOCKING );

	TCPMessage createUDPMessage;
	createUDPMessage.m_header.m_id = MESSAGE_ID_UDP_SOCKET;
	createUDPMessage.m_header.m_size = createUDPMessage.m_message.size();
	createUDPMessage.m_message = "";

	g_theNetworkSystem->SendTCPMessage( createUDPMessage );
	g_theConsole->PrintString( Rgba8::GREEN, "UDP Request sent..." );
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
	SendDisconnectMessage();

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

	if( g_theGame != nullptr )
	{
		g_theGame->BeginFrame();
	}
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
	if( m_socket == nullptr )
		return;

	if( m_frameNum % 1 == 0 && !g_theConsole->IsOpen() )
	{
		InputState inputState = g_theInput->GetInputState();
		SendLargeUDPData( m_socket, m_connectionIP, m_udpSendPort, &inputState, sizeof( InputState ), MESSAGE_ID_INPUT_DATA, m_frameNum );
		//g_theNetworkSystem->SendUDPMessage( inputMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::SendDisconnectMessage()
{
	if( m_socket == nullptr )
		return;

	g_theConsole->PrintString( Rgba8::ORANGE, "Disconnecting..." );
	
	UDPMessage disconnectMessage;
	UDPMessageHeader& header = disconnectMessage.m_header;
	
	memcpy( &header.m_fromAddress[0], &m_connectionIP[0], m_connectionIP.size() );
	header.m_frameNum = m_frameNum;
	header.m_id = MESSAGE_ID_DISCONNECT;
	header.m_key = m_identifier;
	header.m_numMessages = 1;
	header.m_port = m_udpSendPort;
	header.m_seqNo = 0;
	header.m_size = 0;

	g_theNetworkSystem->SendUDPMessage( m_socket, disconnectMessage );
	//SendLargeUDPData( m_socket, m_connectionIP, m_udpSendPort, nullptr, 1, MESSAGE_ID_DISCONNECT, m_frameNum );
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

	g_theNetworkSystem->SendUDPMessage( m_socket, connectionDataRequest );
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::OpenUDPSocket( TCPMessage const& messageToProcess )
{
	m_identifier = messageToProcess.m_header.m_key;

	std::string udpSocketData = messageToProcess.m_message;
	Strings dataSplit = SplitStringOnDelimiter( udpSocketData, '-' );
	m_udpListenPort = atoi( dataSplit[0].c_str() );
	m_udpSendPort = atoi( dataSplit[1].c_str() );

	m_socket = g_theNetworkSystem->OpenUDPPort( m_connectionIP, m_udpListenPort, m_udpSendPort );
	g_theConsole->PrintString( Rgba8::GREEN, "Open UDP Socket at %s listen: %i send: %i", m_connectionIP.c_str(), m_udpListenPort, m_udpSendPort );

	g_theNetworkSystem->DisconnectTCPClient();
	RequestConnectionData();
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessDisconnect()
{
	g_theApp->HandleQuitRequested();
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
		g_theGame->SpawnEntitiesFromSpawnData( connectionData.m_entityData );
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


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessUDPMessages()
{
	if( m_socket == nullptr )
		return;

	std::deque<UDPMessage> udpMessages;
	g_theNetworkSystem->GetUDPMessages( m_socket, udpMessages );
	for( int messageIndex = 0; messageIndex < udpMessages.size(); ++messageIndex )
	{
		UDPMessage newMessage = udpMessages[messageIndex];
		ProcessUDPMessage( newMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessUDPMessage( UDPMessage const& message )
{
	UDPMessageHeader header = message.m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_DISCONNECT: { ProcessDisconnect(); break; }
	case MESSAGE_ID_INPUT_DATA: { ProcessInputData( message ); break; }
	case MESSAGE_ID_ENTITY_DATA: { ProcessEntityData( message ); break; }
	case MESSAGE_ID_CONNECTION_DATA: { ProcessConnectionData( message ); break; }
	case MESSAGE_ID_CAMERA_DATA: { ProcessCameraData( message ); break; }
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::UnpackUDPMessage( UDPMessage const& message )
{
	UDPMessageHeader messageHeader = message.m_header;
	UDPPacket udpPacket( m_packets[messageHeader.m_id] );

	if( udpPacket.m_numMessagesUnpacked != 0 )
	{
		UDPMessageHeader packetHeader = udpPacket.m_header;
		if( packetHeader.m_frameNum < messageHeader.m_frameNum )
		{
			udpPacket = UDPPacket( messageHeader, messageHeader.m_size );
		}
	}
	else
	{
			udpPacket = UDPPacket( messageHeader, messageHeader.m_size );
	}
	
	uint startByte = messageHeader.m_seqNo * MAX_UDP_DATA_SIZE;
	memcpy( &udpPacket.m_data[startByte], &message.m_data[0], Min( MAX_UDP_DATA_SIZE, udpPacket.m_size - startByte ) );
	udpPacket.m_numMessagesUnpacked++;
	m_packets[messageHeader.m_id] = udpPacket;
	
	if( messageHeader.m_isReliable )
	{
		m_socket->SendMessage( message, true );
	}
}