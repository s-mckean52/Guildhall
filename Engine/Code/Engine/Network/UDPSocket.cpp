#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/UDPSocket.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#define TEST_MODE
#ifdef TEST_MODE
	#define LOG_ERROR(...) printf( ( Stringf( __VA_ARGS__ ) + std::string( "\n" ) ).c_str() )
#else
	#define LOG_ERROR(...) g_theConsole->ErrorString( __VA_ARGS__ )
#endif


//---------------------------------------------------------------------------------------------------------
UDPSocket::UDPSocket( std::string const& host, int port )
{
	m_receivePort = static_cast<uint16_t>( port );

	m_toAddress.sin_family = AF_INET;
	m_toAddress.sin_port = htons( (u_short)port );
	m_toAddress.sin_addr.s_addr = inet_addr( host.c_str() );

	m_socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( m_socket == INVALID_SOCKET )
	{
		LOG_ERROR( "Socket instantiation failed, error = '%i'", WSAGetLastError() );
		return;
	}
}


//---------------------------------------------------------------------------------------------------------
UDPSocket::~UDPSocket()
{
	Close();
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
	m_sendToPort = static_cast<uint16_t>( port );

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
		LOG_ERROR( "Socket send failed, error = '%i'", WSAGetLastError() );
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
		LOG_ERROR( "Socket recieve failed, error = '%i'", lastError );
	}
	return result;
}


//---------------------------------------------------------------------------------------------------------
std::string UDPSocket::GetHostData()
{
	constexpr unsigned short size = 128;
    std::array<char, size> hostName;
    const int success = ::gethostname( hostName.data(), size );
    if( success != 0 )
    {
        LOG_ERROR( "Socket failed to get host name, error = %i", WSAGetLastError() );
    }
    hostent* host = ::gethostbyname( hostName.data() );
    memcpy( hostName.data(), inet_ntoa( *( struct in_addr* ) host->h_addr_list[ 0 ] ), size );
    return std::string( hostName.data() );
}
