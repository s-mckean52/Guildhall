#pragma once

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <WinSock2.h>
#include <limits>
#include <string>
#include <array>
constexpr int BufferSize = 512;
typedef std::array<char, BufferSize> Buffer;

class UDPSocket
{
public:

public:
	UDPSocket( std::string const& host, int port );
	~UDPSocket();

	bool	IsValid() const { return m_socket != INVALID_SOCKET; }

	void	Close();
	void	Bind( int port );
	int		Send( int length );
	int		Receive();

	std::string	GetHostData();
	uint16_t	GetSendToPort() { return m_sendToPort; }

	Buffer& SendBuffer()	{ return m_sendBuffer; }
	Buffer& ReceiveBuffer() { return m_recieveBuffer; }

private:
	uint16_t m_sendToPort = 0;
	uint16_t m_receivePort = 0;

	Buffer		m_sendBuffer;
	Buffer		m_recieveBuffer;
	sockaddr_in m_toAddress;
	sockaddr_in m_bindAddress;
	SOCKET		m_socket		= INVALID_SOCKET;
};