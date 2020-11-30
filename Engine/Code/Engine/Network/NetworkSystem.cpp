#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Network/TCPServer.hpp"
#include "Engine/Network/TCPClient.hpp"
#include "Engine/Network/UDPSocket.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <array>

#pragma comment( lib, "Ws2_32.lib" )

//---------------------------------------------------------------------------------------------------------
NetworkSystem::NetworkSystem()
{
}


//---------------------------------------------------------------------------------------------------------
NetworkSystem::~NetworkSystem()
{
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::StartUp()
{
	WSADATA wsaData;
	WORD wVersion MAKEWORD( 2, 2 );
	int iResult = WSAStartup( wVersion, &wsaData );
	
	if( iResult != 0 )
	{
		g_theConsole->ErrorString( "Call to WSAStartup failed %i", WSAGetLastError() );
	}
	g_theEventSystem->SubscribeEventCallbackMethod( "start_tcp_server", this, &NetworkSystem::start_tcp_server );
	g_theEventSystem->SubscribeEventCallbackMethod( "stop_tcp_server", this, &NetworkSystem::stop_tcp_server );
	g_theEventSystem->SubscribeEventCallbackMethod( "send_message", this, &NetworkSystem::send_message );
	g_theEventSystem->SubscribeEventCallbackMethod( "connect", this, &NetworkSystem::client_connect );
	g_theEventSystem->SubscribeEventCallbackMethod( "disconnect", this, &NetworkSystem::client_disconnect );
	g_theEventSystem->SubscribeEventCallbackMethod( "open_udp_port", this, &NetworkSystem::open_udp_port );
	g_theEventSystem->SubscribeEventCallbackMethod( "close_udp_port", this, &NetworkSystem::close_udp_port );
	g_theEventSystem->SubscribeEventCallbackMethod( "send_udp_message", this, &NetworkSystem::send_udp_message );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::BeginFrame()
{
	//UDPReadMessages();
	SendReliableMessages();

	if( m_tcpServer == nullptr && m_mode == TCPMODE_SERVER )
	{
		CreateTCPServer( SocketMode::NONBLOCKING );
	}
	else if( m_mode == TCPMODE_SERVER )
	{	
		if( !m_clientSocket.IsValid() )
		{
			m_clientSocket = m_tcpServer->Accept();
		}
		else if( m_clientSocket.IsDataAvailable() )
		{
			TCPData buf;
			buf = m_clientSocket.Recieve();

			if( buf.GetLength() == TCPData::DATA_PENDING )
			{
				return;
			}

			std::string data( buf.GetData(), buf.GetLength() );
			TCPMessageHeader* messageHeader = reinterpret_cast<TCPMessageHeader*>( &data[0] );
			TCPMessage serverListenMessage;
			serverListenMessage.m_header = *messageHeader;
			serverListenMessage.m_message = std::string( &data[sizeof( TCPMessageHeader )] );

			if( messageHeader->m_id == 3 )
			{
				g_theConsole->PrintString( Rgba8::GREEN, "Client %s is shutting down...", m_clientSocket.GetAddress().c_str() );
				m_clientSocket.ShutDown();
				m_clientSocket.Close();
			}
			else if( buf.GetLength() > 0 )
			{
				//g_theConsole->PrintString( Rgba8::WHITE, "Client: "+ serverListenMessage.m_message );
				AppendTCPMessage( serverListenMessage );
			}
		}
	}
	else if( m_mode == TCPMODE_CLIENT )
	{	
		if( m_clientSocket.IsDataAvailable() )
		{
			TCPData buf;
			buf = m_clientSocket.Recieve();

			if( buf.GetLength() == TCPData::DATA_PENDING )
			{
				return;
			}

			std::string data( buf.GetData(), buf.GetLength() );
			TCPMessageHeader* messageHeader = reinterpret_cast<TCPMessageHeader*>( &data[0] );
			TCPMessage serverListenMessage;
			serverListenMessage.m_header = *messageHeader;
			serverListenMessage.m_message = std::string( &data[sizeof(TCPMessageHeader)] );


			if( messageHeader->m_id == 3 )
			{
				g_theConsole->PrintString( Rgba8::GREEN, "Host Server is shutting down..." );
				m_clientSocket.ShutDown();
				m_clientSocket.Close();
			}
			else if( buf.GetLength() > 0 )
			{
				AppendTCPMessage( serverListenMessage );
				//g_theConsole->PrintString( Rgba8::WHITE, "Server: " + serverListenMessage.m_message );
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::EndFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::ShutDown()
{
	int iResult = WSACleanup();
	if( iResult == SOCKET_ERROR )
	{
		g_theConsole->ErrorString( "Winsock cleanup failed %i", WSAGetLastError() );
	}

	for( int udpSocketIndex = 0; udpSocketIndex < m_UDPSockets.size(); ++udpSocketIndex )
	{
		UDPSocket* udpSocket = m_UDPSockets[udpSocketIndex];
		udpSocket->StopThreads();

		delete m_UDPSockets[udpSocketIndex];
		m_UDPSockets[udpSocketIndex] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CreateTCPServer( SocketMode mode )
{
	if( m_tcpServer != nullptr )
	{
		delete m_tcpServer;
		m_tcpServer = nullptr;
	}

	m_tcpServer = new TCPServer( mode );
	m_tcpServer->SetListenPort( 48000 );
	m_tcpServer->SetIsListening( true );
	m_tcpServer->Bind();
	m_tcpServer->Listen();

	m_mode = TCPMODE_SERVER;
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CreateTCPClient()
{
	if( m_tcpClient != nullptr )
	{
		delete m_tcpClient;
		m_tcpClient = nullptr;
	}

	m_tcpClient = new TCPClient();
	m_mode = TCPMODE_CLIENT;
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::DisconnectTCPClient()
{
	SendDisconnectMessage();
	m_clientSocket.ShutDown();
	m_clientSocket.Close();

	m_mode = TCPMODE_INVALID;
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CloseTCPServer()
{
	SendDisconnectMessage();
	m_clientSocket.ShutDown();
	m_clientSocket.Close();

	m_mode = TCPMODE_INVALID;
}


//---------------------------------------------------------------------------------------------------------
std::string NetworkSystem::GetTCPSocketAddress()
{
	return m_clientSocket.GetAddress();
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::ConnectTCPClient( std::string const& ipAddress, uint16_t portNum, SocketMode socketMode )
{
	m_clientSocket = m_tcpClient->Connect( ipAddress, portNum, socketMode );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::SendTCPMessage( TCPMessage tcpMessageToSend )
{
	std::array<char, 256> buffer;
	memcpy( &buffer, &tcpMessageToSend.m_header, sizeof( TCPMessageHeader ) );

	TCPMessageHeader messageHeader = tcpMessageToSend.m_header;
	std::string message = tcpMessageToSend.m_message;
	
	for( int charIndex = 0; charIndex < message.size(); ++charIndex )
	{
		buffer[sizeof(TCPMessageHeader) + charIndex] = message[charIndex];
	}
	if( m_clientSocket.IsValid() )
	{
		m_clientSocket.Send( &buffer[0], ( messageHeader.m_size + sizeof( TCPMessageHeader ) ) );
	}
	else
	{
		g_theConsole->ErrorString( "TCP Message failed to send: Client Socket is invalid" );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::SendDisconnectMessage()
{
	if( !m_clientSocket.IsValid() )
		return;

	TCPMessageHeader messageHeader;
	messageHeader.m_id = 3;
	messageHeader.m_size = 4;
	m_clientSocket.Send( (const char*)&messageHeader, sizeof( TCPMessageHeader ) );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::AppendTCPMessage( TCPMessage const& tcpMessage )
{
	m_tcpMessages.push_back( tcpMessage );
}


//---------------------------------------------------------------------------------------------------------
bool NetworkSystem::GetTCPMessage( TCPMessage& out_message )
{
	if( m_tcpMessages.size() > 0 )
	{
		out_message = m_tcpMessages.front();
		m_tcpMessages.pop_front();
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::SendReliableMessages()
{
	for( int udpSocketIndex = 0; udpSocketIndex < m_UDPSockets.size(); ++udpSocketIndex )
	{
		m_UDPSockets[udpSocketIndex]->SendReliableMessages();
	}
}


//---------------------------------------------------------------------------------------------------------
UDPSocket* NetworkSystem::OpenUDPPort( std::string const& ipAddress, int bindPort, int sendToPort )
{
	CloseUDPPort( bindPort );

	UDPSocket* newSocket = new UDPSocket( this, ipAddress, bindPort, sendToPort );
	m_UDPSockets.push_back( newSocket );

	return newSocket;
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CloseUDPPort( int bindPort )
{
	for( int udpSocketIndex = 0; udpSocketIndex < m_UDPSockets.size(); ++udpSocketIndex )
	{
		UDPSocket* udpSocket = m_UDPSockets[udpSocketIndex];
		if( udpSocket->GetReceivePort() == bindPort )
		{
			udpSocket->StopThreads();

			m_UDPSockets[udpSocketIndex] = m_UDPSockets[m_UDPSockets.size() - 1];
			m_UDPSockets.pop_back();
			delete udpSocket;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::SendUDPMessage( UDPSocket* socket, UDPMessage const& message )
{
	if( socket != nullptr )
	{
		socket->SendMessage( message );
	}
	else
	{
		g_theConsole->ErrorString( "Must have an open UDP port to send a message" );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::UDPReadMessages( UDPSocket* socket )
{
	UDPMessage message;
	while( socket->ReadMessage( message ) )
	{
		m_udpMessages.push_back( message );
		//g_theConsole->PrintString( Rgba8::GREEN, "%s:%i said: \"%s\"", message.m_header.m_fromAddress, message.m_header.m_port, message.m_data );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::GetUDPMessages( UDPSocket* socket, std::deque<UDPMessage>& out_messages )
{
	socket->GetMessages( out_messages );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::start_tcp_server( EventArgs* args )
{
	int port = args->GetValue( "port", 48000 );

	CreateTCPServer( SocketMode::NONBLOCKING );
	m_tcpServer->SetListenPort( port );
	m_tcpServer->SetIsListening( true );
	m_tcpServer->Bind();
	m_tcpServer->Listen();

	m_mode = TCPMODE_SERVER;

	g_theConsole->PrintString( Rgba8::GREEN, "TCPServer listening on port %i", port );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::stop_tcp_server( EventArgs* args )
{
	UNUSED( args );

	g_theConsole->PrintString( Rgba8::GREEN, "TCPServer is no longer listening on port %i", m_tcpServer->GetListenPort() );
	
	CloseTCPServer();
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::send_message( EventArgs* args )
{
	std::array<char, 256> buffer;
	std::string message = args->GetValue( "message", "" );

	TCPMessageHeader* headerPtr = reinterpret_cast<TCPMessageHeader*>( &buffer[0] );
	headerPtr->m_id = 2;
	headerPtr->m_size = static_cast<uint16_t>( message.size() );
	
	for( int charIndex = 0; charIndex < message.size(); charIndex++ )
	{
		buffer[4 + charIndex] = message[charIndex];
	}
	if( m_clientSocket.IsValid() )
	{
		m_clientSocket.Send( &buffer[0], ( headerPtr->m_size + 4 ) );
		g_theConsole->PrintString( Rgba8::GREEN, "Sent Message: %s", message.c_str() );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::client_connect( EventArgs* args )
{
	std::string input = args->GetValue( "ip", ":48000" );
	Strings split = SplitStringOnDelimiter( input, ':' );
	if( split.size() > 2 )
	{
		g_theConsole->ErrorString( "Bad ip and port input" );
		return;
	}

	g_theConsole->PrintString( Rgba8::GREEN, "Connecting to %s...", input.c_str() );
	g_theNetworkSystem->CreateTCPClient();
	g_theNetworkSystem->ConnectTCPClient( split[0], static_cast<uint16_t>( atoi( split[1].c_str() ) ), SocketMode::NONBLOCKING);

	m_mode = TCPMODE_CLIENT;

}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::client_disconnect( EventArgs* args )
{
	UNUSED( args );

	g_theConsole->PrintString( Rgba8::GREEN, "Disconnecting from %s...", m_clientSocket.GetAddress().c_str() );
	DisconnectTCPClient();
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::open_udp_port( EventArgs* args )
{
	std::string ipAddress = args->GetValue( "ip", "127.0.0.1" );
	int bindPort = args->GetValue( "bindPort", 48000 );
	int sendToPort = args->GetValue( "sendToPort", 48001 );

	g_theConsole->PrintString( Rgba8::GREEN, "Opening UDP Port on %i and sending to on %i...", bindPort, sendToPort );

	OpenUDPPort( ipAddress, bindPort, sendToPort );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::send_udp_message( EventArgs* args )
{
	std::string message = args->GetValue( "message", "" );

	g_theConsole->PrintString( Rgba8::GREEN, "Sending Message..." );

	std::string hostData = m_UDPSockets[0]->GetHostData();
	uint16_t sendToPort = static_cast<uint16_t>( m_UDPSockets[0]->GetSendToPort() );
	
	UDPMessage messageToSend;
	messageToSend.m_header.m_port = sendToPort;
	memcpy( messageToSend.m_header.m_fromAddress, &hostData, 16 );
	memcpy(messageToSend.m_data, &message, message.size());
	SendUDPMessage( m_UDPSockets[0], messageToSend );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::close_udp_port( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );

	g_theConsole->PrintString( Rgba8::GREEN, "Closing port on %i...", bindPort );

	CloseUDPPort( bindPort );
}