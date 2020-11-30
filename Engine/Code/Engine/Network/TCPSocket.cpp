#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include <array>

//---------------------------------------------------------------------------------------------------------
// TCPData
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
TCPData::TCPData()
{
}


//---------------------------------------------------------------------------------------------------------
TCPData::TCPData( size_t length, char* data )
{
	m_length	= length;
	m_data		= data;
}


//---------------------------------------------------------------------------------------------------------
TCPData::~TCPData()
{
	m_length	= 0;
	m_data		= nullptr;
}


//---------------------------------------------------------------------------------------------------------
// TCPSocket
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
TCPSocket::TCPSocket()
	: m_timeval{ 0l, 0l }
{
	FD_ZERO( &m_fdSet );
	m_buffer = new char[m_bufferSize];
}


//---------------------------------------------------------------------------------------------------------
TCPSocket::TCPSocket( SOCKET socket, SocketMode mode, size_t bufferSize )
	: m_socket( socket )
	, m_mode( mode )
	, m_bufferSize( bufferSize )
	, m_timeval{ 0l, 0l }
{
	FD_ZERO( &m_fdSet );
	m_buffer = new char[m_bufferSize];
}


//---------------------------------------------------------------------------------------------------------
TCPSocket::TCPSocket(TCPSocket const& copyFrom)
	: m_mode( copyFrom.m_mode )
	, m_socket( copyFrom.m_socket )
	, m_fdSet( copyFrom.m_fdSet )
	, m_timeval( copyFrom.m_timeval )
	, m_bufferSize( copyFrom.m_bufferSize )
	, m_receiveSize( copyFrom.m_receiveSize )
	, m_buffer( copyFrom.m_buffer )
{
	if( m_bufferSize > 0 )
	{
		m_buffer = new char[m_bufferSize];
	}
}


//---------------------------------------------------------------------------------------------------------
TCPSocket::~TCPSocket()
{
	m_mode			= SocketMode::INVALID;
	m_socket		= INVALID_SOCKET;
	m_bufferSize	= 0;
	m_receiveSize	= 0;
	
	FD_ZERO( &m_fdSet );
	delete[] m_buffer;
}


//---------------------------------------------------------------------------------------------------------
void TCPSocket::Send( char const* data, size_t length )
{
	int iResult = send( m_socket, data, static_cast<int>( length ), 0 );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Call to send failed %i", WSAGetLastError() );
		Close();
	}
	else if( iResult < static_cast<int>( length ) )
	{
		g_theConsole->ErrorString( "Requested %i bytes to be sent. Only %i bytes were sent.", length, iResult );
		Close();
	}
}


//---------------------------------------------------------------------------------------------------------
TCPData TCPSocket::Recieve()
{
	int iResult = ::recv( m_socket, m_buffer, static_cast<int>( m_bufferSize ), 0 );
	if (iResult == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if( error == WSAEWOULDBLOCK && m_mode == SocketMode::NONBLOCKING )
		{
			return TCPData{ TCPData::DATA_PENDING, NULL };
		}
		else
		{
			g_theConsole->ErrorString( "Call to recv failed %i", WSAGetLastError() );
			Close();
		}
	}
	return TCPData{ static_cast<size_t>( iResult ), m_buffer };
}


//---------------------------------------------------------------------------------------------------------
void TCPSocket::ShutDown()
{
	if( !IsValid() )
		return;

	int iResult = shutdown( m_socket, SD_SEND );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Call to shutdown failed %i", WSAGetLastError() );
		Close();
	}
	//m_socket = INVALID_SOCKET;
}


//---------------------------------------------------------------------------------------------------------
void TCPSocket::Close()
{
	if( !IsValid() )
		return;

	int iResult = closesocket( m_socket );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Call to closesocket failed %i", WSAGetLastError() );
	}
	m_socket = INVALID_SOCKET;
}


//---------------------------------------------------------------------------------------------------------
bool TCPSocket::IsDataAvailable()
{
	if( !IsValid() )
		return false;

	if( m_mode == SocketMode::NONBLOCKING )
	{
		FD_ZERO( &m_fdSet );
		FD_SET( m_socket, &m_fdSet );
		int iResult = select( 0, &m_fdSet, NULL, NULL, &m_timeval );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( "Call to select failed %i", WSAGetLastError() );
			closesocket( m_socket );
		}
		return FD_ISSET( m_socket, &m_fdSet );
	}
	else
	{
		g_theConsole->ErrorString( "IsDataAvailable only works on nonblocking mode" );
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------
std::string TCPSocket::GetAddress() const
{
	std::array<char, 64> addressStr;

	sockaddr clientAddr;
	int addrSize = sizeof( clientAddr );
	int iResult = getpeername( m_socket, &clientAddr, &addrSize );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Call to getpeername failed %i", WSAGetLastError() );
	}

	DWORD outlen = static_cast<DWORD>( addressStr.size() );
	iResult = WSAAddressToStringA( &clientAddr, addrSize, NULL, &addressStr[0], &outlen );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Call to getpeername failed %i", WSAGetLastError() );
	}
	addressStr[outlen] = NULL;

	return std::string( &addressStr[0] );
}


//---------------------------------------------------------------------------------------------------------
void TCPSocket::SetSocketMode( SocketMode mode )
{
	m_mode = mode;
	if( m_mode == SocketMode::INVALID )
	{
		unsigned long blockingMode = static_cast<unsigned long>( m_mode );
		int iResult = ioctlsocket( m_socket, FIONBIO, &blockingMode );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( "Call to ioctlsocket failed %i", WSAGetLastError() );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
TCPSocket& TCPSocket::operator=( const TCPSocket& src )
{
	if( this != &src )
	{
		m_mode			= src.m_mode;
		m_bufferSize	= src.m_bufferSize;
		m_receiveSize	= src.m_receiveSize;
		m_socket		= src.m_socket;

		if( m_bufferSize > 0 )
		{
			m_buffer = new char[m_bufferSize];
		}
	}
	return *this;
}