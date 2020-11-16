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
	m_size = size;
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
