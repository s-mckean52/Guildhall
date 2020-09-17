#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Network/TCPClient.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include <ws2tcpip.h>


//---------------------------------------------------------------------------------------------------------
TCPClient::TCPClient()
	: m_mode( SocketMode::BLOCKING )
{
}


//---------------------------------------------------------------------------------------------------------
TCPClient::~TCPClient()
{
	m_mode = SocketMode::INVALID;
}


//---------------------------------------------------------------------------------------------------------
TCPSocket TCPClient::Connect( std::string const& host, uint16_t port, SocketMode mode )
{
	m_mode = mode;
	
	struct addrinfo addrHintsIn;
	struct addrinfo* addrOut = nullptr;

	ZeroMemory( &addrHintsIn, sizeof( addrHintsIn ) );
	addrHintsIn.ai_family = AF_INET;
	addrHintsIn.ai_socktype = SOCK_STREAM;
	addrHintsIn.ai_protocol = IPPROTO_TCP;

	std::string serverPort( Stringf( "%d", port ) );
	int iResult = getaddrinfo( host.c_str(), serverPort.c_str(), &addrHintsIn, &addrOut );
	if( iResult != 0 )
	{
		g_theConsole->ErrorString( "Call to getaddrinfo failed %i", iResult );
	}

	SOCKET connectionSocket = socket( addrOut->ai_family, addrOut->ai_socktype, addrOut->ai_protocol );
	if( connectionSocket == INVALID_SOCKET )
	{
		g_theConsole->ErrorString( "Call to socket failed %i", WSAGetLastError() );
	}

	iResult = connect( connectionSocket, addrOut->ai_addr, static_cast<int>( addrOut->ai_addrlen ) );
	if( iResult == SOCKET_ERROR )
	{
		closesocket( connectionSocket );
		connectionSocket = INVALID_SOCKET;
	}
	freeaddrinfo( addrOut );

	if( connectionSocket == INVALID_SOCKET )
	{
		g_theConsole->ErrorString( "Connection Failed" );
	}

	if( m_mode == SocketMode::NONBLOCKING )
	{
		u_long winsockmode = 1;
		iResult = ioctlsocket(connectionSocket, FIONBIO, &winsockmode);
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( "IOCTL failed on client listen socket %i", WSAGetLastError() );
			closesocket( connectionSocket );
		}
	}
	return TCPSocket( connectionSocket, m_mode );
}