#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/RemoteServer.hpp"

//---------------------------------------------------------------------------------------------------------
RemoteServer::RemoteServer( std::string const& ipAddress, std::string const& portNum )
{
	g_theNetworkSystem->CreateTCPClient();

	uint16_t portAsNum = static_cast<uint16_t>( atoi( portNum.c_str() ) );
	g_theNetworkSystem->ConnectTCPClient( ipAddress, portAsNum, SocketMode::NONBLOCKING );

	TCPMessage createUDPMessage;

	m_udpListenPort = g_RNG->RollRandomIntInRange( 48000, 49000 );
	createUDPMessage.m_message = ToString( m_udpListenPort );

	createUDPMessage.m_header.m_id = MESSAGE_ID_UDP_REQUEST;
	createUDPMessage.m_header.m_size = createUDPMessage.m_message.size();

	g_theNetworkSystem->SendTCPMessage( createUDPMessage );
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::StartUp( GameType gameType )
{
	UNUSED( gameType );
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ShutDown()
{
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::BeginFrame()
{
	ProcessTCPMessage( g_theNetworkSystem->GetTCPMessage() );
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::EndFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::Update()
{

}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::ProcessTCPMessage( TCPMessage* messageToProcess )
{
	if( messageToProcess == nullptr )
		return;

	TCPMessageHeader header = messageToProcess->m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_UDP_SOCKET: { OpenUDPSocket( messageToProcess->m_message ); } break;
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void RemoteServer::OpenUDPSocket( std::string const& udpSocketData )
{
	Strings dataSplit = SplitStringOnDelimiter( udpSocketData, ':' );
	m_connectionIP = dataSplit[0];
	m_udpSendPort = atoi( dataSplit[1].c_str() );
	g_theNetworkSystem->OpenUDPPort( m_udpListenPort, m_udpSendPort );
	g_theConsole->PrintString( Rgba8::GREEN, "Open UDP Socket at %s listen on %i send on %i", m_connectionIP.c_str(), m_udpListenPort, m_udpSendPort );
}

