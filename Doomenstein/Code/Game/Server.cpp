#include "Engine/Math/MathUtils.hpp"
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
void Server::SendLargeUDPData( std::string const& ipAddress, int sendToPort, void const* data, uint dataSize, MessageID messageType, uint frameNum )
{
	uint numMessages = static_cast<uint>( ceilf( static_cast<float>( dataSize ) / static_cast<float>( MAX_UDP_DATA_SIZE ) ) );
	
	UDPMessageHeader messageHeader;
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

		g_theNetworkSystem->SendUDPMessage( message );
	}
}


//---------------------------------------------------------------------------------------------------------
void Server::UnpackUDPMessage( UDPMessage const& message )
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
}


//---------------------------------------------------------------------------------------------------------
void Server::ProcessUDPMessages()
{
	std::deque<UDPMessage> udpMessages;
	g_theNetworkSystem->GetUDPMessages( udpMessages );
	for( int messageIndex = 0; messageIndex < udpMessages.size(); ++messageIndex )
	{
		UDPMessage newMessage = udpMessages[messageIndex];
		ProcessUDPMessage( newMessage );
	}
}


//---------------------------------------------------------------------------------------------------------
void Server::ProcessUDPMessage( UDPMessage const& message )
{
	UDPMessageHeader header = message.m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_INPUT_DATA: { ProcessInputData( message ); break; }
	case MESSAGE_ID_ENTITY_DATA: { ProcessEntityData( message ); break; }
	case MESSAGE_ID_CONNECTION_DATA: { ProcessConnectionData( message ); break; }
	case MESSAGE_ID_CAMERA_DATA: { ProcessCameraData( message ); break; }
	default:
		break;
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
	TCPMessageHeader header = messageToProcess.m_header;
	switch( header.m_id )
	{
	case MESSAGE_ID_UDP_SOCKET: { OpenUDPSocket( messageToProcess ); break; }
	default:
		break;
	}
}