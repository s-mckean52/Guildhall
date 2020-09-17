#pragma once

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <string>
#include <limits>
#include <winsock2.h>

#ifdef max
#undef max
#endif

enum class SocketMode
{
	INVALID		= 0,
	BLOCKING	= 1,
	NONBLOCKING	= 2,
};

//---------------------------------------------------------------------------------------------------------
class TCPData
{
public:
	TCPData();
	TCPData( size_t length, char* data );
	~TCPData();

	size_t		GetLength() const	{ return m_length; }
	char const*	GetData() const		{ return m_data; }

	static const size_t DATA_PENDING = std::numeric_limits<size_t>::max();

private:
	size_t	m_length	= 0;
	char*	m_data		= nullptr;
};


//---------------------------------------------------------------------------------------------------------
class TCPSocket
{
public:
	TCPSocket();
	TCPSocket( SOCKET socket, SocketMode mode = SocketMode::BLOCKING, size_t bufferSize = 256 );
	TCPSocket( TCPSocket const& copyFrom );
	~TCPSocket();

	void		Send( char const* data, size_t length );
	TCPData		Recieve();
	void		ShutDown();
	void		Close();

	bool		IsDataAvailable();
	bool		IsValid() const			{ return m_socket != INVALID_SOCKET; }
	size_t		GetBufferSize() const	{ return m_bufferSize; }
	SocketMode	GetSocketMode() const	{ return m_mode; }
	std::string GetAddress() const;

	void		SetSocketMode( SocketMode mode );


	
public:
	TCPSocket& operator=( const TCPSocket& src );

private:
	FD_SET		m_fdSet;
	timeval		m_timeval;

	SOCKET		m_socket	= INVALID_SOCKET;
	SocketMode	m_mode		= SocketMode::INVALID;

	char*	m_buffer		= nullptr;
	size_t	m_bufferSize	= 256;
	size_t	m_receiveSize	= 0;
};