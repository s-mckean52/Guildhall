#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/UDPSocket.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#ifdef TEST_MODE
	#define LOG_ERROR(...) printf( ( Stringf( __VA_ARGS__ ) + std::string( "\n" ) ).c_str() )
#else
	#define LOG_ERROR(...) g_theConsole->ErrorString( __VA_ARGS__ )
#endif


//---------------------------------------------------------------------------------------------------------
UDPSocket::UDPSocket( std::string const& host, int port )
{
	m_toAddress.sin_family = AF_INET;
	m_toAddress.sin_port = htons( (u_short)port );
	m_toAddress.sin_addr.s_addr = inet_addr( host.c_str() );

	m_socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( m_socket == INVALID_SOCKET )
	{
		LOG_ERROR( "Socket instantiation failed, error = '%i'", WSAGetLastError() );
	}
}


//---------------------------------------------------------------------------------------------------------
UDPSocket::~UDPSocket()
{
	Close();
}


//---------------------------------------------------------------------------------------------------------
bool UDPSocket::IsDataAvailable()
{
	FD_ZERO( &m_fdSet );
	FD_SET( m_socket, &m_fdSet );
	int iResult = select( 0, &m_fdSet, NULL, NULL, &m_timeval );
	if (iResult == SOCKET_ERROR)
	{
		LOG_ERROR( "Call to select failed %i", WSAGetLastError() );
		closesocket( m_socket );
	}
	return FD_ISSET( m_socket, &m_fdSet );
}


//---------------------------------------------------------------------------------------------------------
void UDPSocket::Close()
{
	if( m_socket == INVALID_SOCKET )
		return;

	int result = closesocket( m_socket );
	if( result == SOCKET_ERROR )
	{
		LOG_ERROR( "Socket close failed, error = '%i'", WSAGetLastError() );
	}
	m_socket = INVALID_SOCKET;
}


//---------------------------------------------------------------------------------------------------------
void UDPSocket::Bind( int port )
{
	m_bindAddress.sin_family = AF_INET;
	m_bindAddress.sin_port = htons( (u_short)port );
	m_bindAddress.sin_addr.s_addr = htonl( INADDR_ANY );

	int result = ::bind( m_socket, (SOCKADDR*)&m_bindAddress, sizeof( m_bindAddress ) );
	if( result != 0 )
	{
		LOG_ERROR( "Socket bind failed, error = '%i'", WSAGetLastError() );
	}
}


//---------------------------------------------------------------------------------------------------------
int UDPSocket::Send( int length )
{
	int result = ::sendto( m_socket, &m_sendBuffer[0], length, 0, reinterpret_cast<SOCKADDR*>( &m_toAddress ), sizeof( m_toAddress ) );
	if( result == SOCKET_ERROR )
	{
		LOG_ERROR( "Socket bind failed, error = '%i'", WSAGetLastError() );
	}
	return result;
}


//---------------------------------------------------------------------------------------------------------
int UDPSocket::Receive()
{
	sockaddr_in fromAddr;
	int fromLen = sizeof( fromAddr );
	int result = ::recvfrom( m_socket, &m_recieveBuffer[0], static_cast<int>( m_recieveBuffer.size() ), 0, reinterpret_cast<SOCKADDR*>( &fromAddr ), &fromLen );
	if( result == SOCKET_ERROR )
	{
		int lastError = WSAGetLastError();
		LOG_ERROR( "Socket bind failed, error = '%i'", lastError );
	}
	return result;
}
