#include "Client.hpp"
#include "Engine/Network/NetworkMessages.hpp"
#include "Engine/Math/MathUtils.hpp"

//---------------------------------------------------------------------------------------------------------
Client::Client( Server* owner )
	: m_owner( owner )
{
}


//---------------------------------------------------------------------------------------------------------
Client::~Client()
{
}


//---------------------------------------------------------------------------------------------------------
void Client::UnpackUDPMessage( UDPMessage const& message )
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