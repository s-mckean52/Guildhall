#include "Engine/Network/NetworkMessages.hpp"

//---------------------------------------------------------------------------------------------------------
UDPPacket::UDPPacket()
{
	m_data = new unsigned char[0];
}

//---------------------------------------------------------------------------------------------------------
UDPPacket::UDPPacket( UDPMessageHeader const& messageHeader, uint size )
{
	m_header = messageHeader;
	m_size = static_cast<uint16_t>( size );
	m_data = new unsigned char[size];
}

//---------------------------------------------------------------------------------------------------------
UDPPacket::UDPPacket( UDPPacket const& copyFrom )
{
	m_numMessagesUnpacked = copyFrom.m_numMessagesUnpacked;
	m_header = copyFrom.m_header;
	m_size = copyFrom.m_size;

	if( m_data != nullptr )
	{
		delete[] m_data;
		m_data = nullptr;
	}
	m_data = new unsigned char[m_size];
	memcpy( &m_data[0], &copyFrom.m_data[0], m_size );
}

//---------------------------------------------------------------------------------------------------------
UDPPacket::~UDPPacket()
{
	if( m_data != nullptr )
	{
		delete[] m_data;
		m_data = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void UDPPacket::operator=( UDPPacket const& copyFrom )
{
	m_numMessagesUnpacked = copyFrom.m_numMessagesUnpacked;
	m_header = copyFrom.m_header;
	m_size = copyFrom.m_size;

	if( m_data != nullptr )
	{
		delete[] m_data;
		m_data = nullptr;
	}
	m_data = new unsigned char[m_size];
	memcpy( &m_data[0], &copyFrom.m_data[0], m_size );
}


//---------------------------------------------------------------------------------------------------------
bool UDPMessage::operator==( UDPMessage const& message )
{
	if( m_header == message.m_header && m_data == message.m_data )
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool UDPMessageHeader::operator==( UDPMessageHeader const& header )
{
	if(	m_isReliable	== header.m_isReliable	&&
		m_key			== header.m_key			&&
		m_id			== header.m_id			&&
		m_frameNum		== header.m_frameNum	&&
		m_seqNo			== header.m_seqNo		&&
		m_port			== header.m_port		&&
		m_numMessages	== header.m_numMessages	&&
		m_size			== header.m_size		&&
		m_fromAddress	== header.m_fromAddress )
	{
		return true;
	}
	return false;
}
