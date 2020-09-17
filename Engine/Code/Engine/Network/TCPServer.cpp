#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Network/TCPServer.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include <array>


//---------------------------------------------------------------------------------------------------------
TCPServer::TCPServer( SocketMode mode, int port )
	: m_listenPort( port )
	, m_listenSocket( INVALID_SOCKET )
	, m_timeval{ 0l, 0l }
	, m_mode( mode )
{
	FD_ZERO( &m_listenSet );
}


//---------------------------------------------------------------------------------------------------------
TCPServer::~TCPServer()
{
	FD_ZERO( &m_listenSet );
	m_listenPort = INVALID_SOCKET;
}


//---------------------------------------------------------------------------------------------------------
void TCPServer::SetListenPort( int port )
{
	m_listenPort = port;
}


//---------------------------------------------------------------------------------------------------------
void TCPServer::SetIsListening( bool isListening )
{
	m_isListening = isListening;
}

//---------------------------------------------------------------------------------------------------------
void TCPServer::Bind()
{
	if( m_listenSocket == INVALID_SOCKET )
	{
		struct addrinfo addrHintsIn;
		struct addrinfo* addrOut;

		ZeroMemory( &addrHintsIn, sizeof( addrHintsIn ) );
		addrHintsIn.ai_family = AF_INET;
		addrHintsIn.ai_socktype = SOCK_STREAM;
		addrHintsIn.ai_protocol = IPPROTO_TCP;
		addrHintsIn.ai_flags = AI_PASSIVE;
		
		std::string serverPort( Stringf( "%d", m_listenPort ) );
		int iResult = getaddrinfo( NULL, serverPort.c_str(), &addrHintsIn, &addrOut );
		if( iResult != 0 )
		{
			g_theConsole->ErrorString( "Call to getaddrinfo failed %i", iResult );
		}

		m_listenSocket = socket( addrOut->ai_family, addrOut->ai_socktype, addrOut->ai_protocol );
		if( m_listenSocket == INVALID_SOCKET )
		{
			g_theConsole->ErrorString( "Call to socket failed %i", WSAGetLastError() );
		}

		unsigned long blockingMode = 1;
		iResult = ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( "Call to ioctlsocket failed %i", WSAGetLastError() );
		}

		iResult = bind( m_listenSocket, addrOut->ai_addr, static_cast<int>( addrOut->ai_addrlen ) );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( "Call to bind failed %i", WSAGetLastError() );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void TCPServer::Listen()
{
	int iResult = listen( m_listenSocket, SOMAXCONN );
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Call to listen failed %i", WSAGetLastError() );
	}
}


//---------------------------------------------------------------------------------------------------------
void TCPServer::StopListen()
{
	if( m_listenSocket != INVALID_SOCKET )
	{
		int iResult = closesocket( m_listenSocket );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( "Call to closesocket on server listen socket failed %i", WSAGetLastError() );
		}
		m_listenSocket = INVALID_SOCKET;
	}
}


//---------------------------------------------------------------------------------------------------------
TCPSocket TCPServer::Accept()
{
	SOCKET connectionSocket = INVALID_SOCKET;
	if( m_mode == SocketMode::NONBLOCKING )
	{
		FD_ZERO( &m_listenSet );
		FD_SET( m_listenSocket, &m_listenSet );
		int iResult = select( 0, &m_listenSet, NULL, NULL, &m_timeval );
		if( iResult == SOCKET_ERROR )
		{
			g_theConsole->ErrorString( " Call to select failed %i", WSAGetLastError() );
			closesocket( connectionSocket );
			return TCPSocket();
		}
	}
	
	if( m_mode == SocketMode::BLOCKING || ( m_mode == SocketMode::NONBLOCKING && FD_ISSET( m_listenSocket, &m_listenSet ) ) )
	{
		connectionSocket = accept( m_listenSocket, NULL, NULL );
		if( connectionSocket == INVALID_SOCKET )
		{
			g_theConsole->ErrorString( "Call to accept failed %i", WSAGetLastError() );
			closesocket( connectionSocket );
			return TCPSocket();
		}
		TCPSocket tcpSocket( connectionSocket, SocketMode::NONBLOCKING );
		g_theConsole->PrintString( Rgba8::GREEN, "Client connected from %s", tcpSocket.GetAddress().c_str() );
		return tcpSocket;
	}
	return TCPSocket();
}