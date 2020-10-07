#pragma once

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include <ws2tcpip.h>
#include <winsock2.h>
#include <cstdint>
#include <string>


//---------------------------------------------------------------------------------------------------------
class TCPServer
{
public:
	TCPServer( SocketMode mode, int port = 48000 );
	~TCPServer();

	int			GetListenPort() const		{ return m_listenPort; }
	bool		GetIsListening() const		{ return m_isListening; }
	SocketMode	GetSocketMode() const		{ return m_mode; }

	void SetListenPort( int port );
	void SetIsListening( bool isListening );

	void		Bind();
	void		Listen();
	void		StopListen();
	TCPSocket	Accept();

private:
	timeval m_timeval;
	FD_SET	m_listenSet;
	int		m_listenPort	= -1;
	bool	m_isListening	= false;
	SOCKET	m_listenSocket	= INVALID_SOCKET;
	SocketMode m_mode		= SocketMode::INVALID;
};