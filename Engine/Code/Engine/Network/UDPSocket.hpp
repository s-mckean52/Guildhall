#pragma once

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <WinSock2.h>
#include <limits>
#include <string>
#include <array>

class UDPSocket
{
public:
	static const int BufferSize = 512;
	using Buffer = std::array<char, BufferSize>;

public:
	UDPSocket( std::string const& host, int port );
	~UDPSocket();

	bool	IsValid() const { return m_socket != INVALID_SOCKET; }
	bool	IsDataAvailable();

	void	Close();
	void	Bind( int port );
	int		Send( int length );
	int		Receive();

	Buffer& SendBuffer()	{ return m_sendBuffer; }
	Buffer& ReceiveBuffer() { return m_recieveBuffer; }

private:
	FD_SET		m_fdSet;
	timeval		m_timeval;

	Buffer		m_sendBuffer;
	Buffer		m_recieveBuffer;
	sockaddr_in m_toAddress;
	sockaddr_in m_bindAddress;
	SOCKET		m_socket		= INVALID_SOCKET;
};