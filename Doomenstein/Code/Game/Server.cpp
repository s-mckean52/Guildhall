#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Game.hpp"
#include "Game/Server.hpp"
#include "Game/Client.hpp"


//---------------------------------------------------------------------------------------------------------
Server::Server()
{
}


//---------------------------------------------------------------------------------------------------------
Server::~Server()
{
}


//---------------------------------------------------------------------------------------------------------
Game* Server::GetGame() const
{
	return g_theGame;
}


//---------------------------------------------------------------------------------------------------------
void Server::AddClient( Client* client )
{
	m_clients.push_back( client );
}


//---------------------------------------------------------------------------------------------------------
void Server::SendLargeUDPData( UDPSocket* socket, std::string const& ipAddress, int sendToPort, void const* data, uint dataSize, MessageID messageType, uint frameNum, bool isReliable )
{
	uint numMessages = static_cast<uint>( ceilf( static_cast<float>( dataSize ) / static_cast<float>( MAX_UDP_DATA_SIZE ) ) );
	
	UDPMessageHeader messageHeader;
	messageHeader.m_isReliable = isReliable;
	messageHeader.m_id = messageType;
	messageHeader.m_key = m_identifier;
	messageHeader.m_frameNum = frameNum;
	messageHeader.m_port = sendToPort;
	messageHeader.m_numMessages = numMessages;
	messageHeader.m_size = dataSize;
	memcpy( &messageHeader.m_fromAddress[0], &ipAddress[0], 16 );

	unsigned char* dataAsChar = (unsigned char*)data;
	for( uint messageIndex = 0; messageIndex < numMessages; ++messageIndex )
	{
		UDPMessage message;
		message.m_header = messageHeader;
		message.m_header.m_seqNo = messageIndex;

		uint currByte = messageIndex * MAX_UDP_DATA_SIZE;
		memcpy( &message.m_data, &dataAsChar[currByte], MAX_UDP_DATA_SIZE );

		g_theNetworkSystem->SendUDPMessage( socket, message );
	}
}


//---------------------------------------------------------------------------------------------------------
void Server::ProcessTCPMessages()
{
	for( ;; )
	{
		TCPMessage newMessage;
		if( !g_theNetworkSystem->GetTCPMessage( newMessage ) )
			break;

		ProcessTCPMessage( newMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void Server::ProcessTCPMessage( TCPMessage const& messageToProcess )
{
	g_theConsole->PrintString( Rgba8::GREEN, "TCP Message Received..." );

	TCPMessageHeader header = messageToProcess.m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_UDP_SOCKET: { OpenUDPSocket( messageToProcess ); break; }
	default:
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void Server::SendReliableUpdates()
{
	for( int i = 0; i < m_clients.size(); ++i )
	{
		m_clients[i]->SendReliableWorldData();
	}
}
