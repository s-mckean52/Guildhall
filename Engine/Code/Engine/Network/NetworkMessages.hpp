#pragma once
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
struct TCPMessageHeader
{
	uint16_t m_id;
	uint16_t m_size;
	int m_key;
};


//---------------------------------------------------------------------------------------------------------
struct TCPMessage
{
	TCPMessageHeader	m_header;
	std::string			m_message;
};


//---------------------------------------------------------------------------------------------------------
struct UDPMessageHeader
{
	bool		m_isReliable = false;
	int			m_key = 0;
	uint16_t	m_id = 0;
	int			m_frameNum = 0;
	uint16_t	m_seqNo = 0;
	uint16_t	m_port = 0;
	uint16_t	m_numMessages = 0;
	uint16_t	m_size = 0;
	char		m_fromAddress[16];

	bool operator==( UDPMessageHeader const& header );
};
constexpr uint MAX_UDP_DATA_SIZE = MAX_UDP_MESSAGE_SIZE - sizeof( UDPMessageHeader );

//---------------------------------------------------------------------------------------------------------
struct UDPMessage
{
public:
	UDPMessageHeader	m_header;
	unsigned char m_data[ MAX_UDP_DATA_SIZE ] = { 0 };

	UDPMessage() = default;
	~UDPMessage() {};

	bool operator==( UDPMessage const& message );
};


//---------------------------------------------------------------------------------------------------------
struct UDPPacket
{
public:
	uint16_t			m_numMessagesUnpacked = 0;
	uint16_t			m_size = 0;
	UDPMessageHeader	m_header;
	unsigned char*		m_data = nullptr;


	UDPPacket();
	UDPPacket( UDPPacket const& copyFrom );
	UDPPacket( UDPMessageHeader const& messageHeader, uint size );
	~UDPPacket();

	void operator=( UDPPacket const& copyFrom );

	bool IsReadyToRead() const { return m_header.m_numMessages == m_numMessagesUnpacked; }
};