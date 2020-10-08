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
	UDPReadMessages();

	if( m_tcpServers.empty() && m_mode == TCPMODE_SERVER )
	{
		CreateTCPServer( SocketMode::NONBLOCKING );
		m_tcpServers.front()->Bind();
		m_tcpServers.front()->Listen();
	}
	else if( m_mode == TCPMODE_SERVER )
	{	
		if( !m_clientSocket.IsValid() )
		{
			m_clientSocket = m_tcpServers.front()->Accept();
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
			serverListenMessage.m_message = std::string( &data[4] );

			if( messageHeader->m_id == 3 )
			{
				g_theConsole->PrintString( Rgba8::GREEN, "Client %s is shutting down...", m_clientSocket.GetAddress().c_str() );
				m_clientSocket.ShutDown();
				m_clientSocket.Close();
			}
			else if( buf.GetLength() > 0 )
			{
				g_theConsole->PrintString( Rgba8::WHITE, "Client: "+ serverListenMessage.m_message );
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
			serverListenMessage.m_message = std::string( &data[4] );


			if( messageHeader->m_id == 3 )
			{
				g_theConsole->PrintString( Rgba8::GREEN, "Host Server is shutting down..." );
				m_clientSocket.ShutDown();
				m_clientSocket.Close();
			}
			else if( buf.GetLength() > 0 )
			{
				g_theConsole->PrintString( Rgba8::WHITE, "Server: " + serverListenMessage.m_message );
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

	if( m_UDPSocket != nullptr )
	{
		CloseUDPPort( 48000 );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CreateTCPServer( SocketMode mode )
{
	m_tcpServers.push_back( new TCPServer( mode ) );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CreateTCPClient()
{
	m_tcpClients.push_back( new TCPClient() );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::SendDisconnectMessage()
{
	if( !m_clientSocket.IsValid() )
		return;

	TCPMessageHeader messageHeader;
	messageHeader.m_id = 3;
	messageHeader.m_size = 4;
	m_clientSocket.Send( (const char*)&messageHeader, 4 );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::OpenUDPPort( int bindPort, int sendToPort )
{
	if( m_UDPSocket != nullptr )
	{
		CloseUDPPort( 48000 );
	}

	m_isUDPSocketQuitting = false;
	m_UDPSocket = new UDPSocket( "127.0.0.1", bindPort );
	m_UDPSocket->Bind( sendToPort );

	m_UDPReadThread = std::thread( &NetworkSystem::UDPReceiveMessagesJob, this, m_UDPSocket );
	m_UDPSendThread = std::thread( &NetworkSystem::UDPSendMessagesJob, this, m_UDPSocket );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::CloseUDPPort( int bindPort )
{
	UNUSED( bindPort );

	if( m_UDPSocket == nullptr )
		return;

	m_isUDPSocketQuitting = true;

	delete m_UDPSocket;
	m_UDPSocket = nullptr;

	m_UDPReadThread.join();
	m_UDPSendThread.join();

}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::SendUDPMessage( std::string const& ipAddress, uint16_t port, uint16_t id, uint16_t sequenceNum, std::string const& message )
{
	if( m_UDPSocket != nullptr && m_UDPSocket->IsValid() )
	{
		UDPMessage messageToSend;

		uint ipLength = Min( static_cast<uint>( ipAddress.length() ), 15 );
		memcpy( messageToSend.m_header.m_fromAddress, ipAddress.c_str(), ipLength );
		messageToSend.m_header.m_fromAddress[ipLength] = '\0';

		messageToSend.m_header.m_port = port;

		messageToSend.m_header.m_id = id;
		messageToSend.m_header.m_seqNo = sequenceNum;
		messageToSend.m_header.m_size = static_cast<uint16_t>( message.length() );
		messageToSend.m_message = message;

		m_UDPMessagesToSend.Push( messageToSend );
	}
	else
	{
		g_theConsole->ErrorString( "Must have an open UDP port to send a message" );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::UDPReadMessages()
{
	UDPMessage message;
	while( m_UDPMessagesToReceive.Pop( message ) )
	{
		g_theConsole->PrintString( Rgba8::YELLOW, "%s:%i said: \"%s\"", message.m_header.m_fromAddress, message.m_header.m_port, message.m_message.c_str() );
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::start_tcp_server( EventArgs* args )
{
	int port = args->GetValue( "port", 48000 );

	CreateTCPServer( SocketMode::NONBLOCKING );
	m_tcpServers.front()->SetListenPort( port );
	m_tcpServers.front()->SetIsListening( true );
	m_tcpServers.front()->Bind();
	m_tcpServers.front()->Listen();

	m_mode = TCPMODE_SERVER;

	g_theConsole->PrintString( Rgba8::GREEN, "TCPServer listening on port %i", port );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::stop_tcp_server( EventArgs* args )
{
	UNUSED( args );

	g_theConsole->PrintString( Rgba8::GREEN, "TCPServer is no longer listening on port %i", m_tcpServers.front()->GetListenPort() );
	
	SendDisconnectMessage();
	m_clientSocket.ShutDown();
	m_clientSocket.Close();

	m_mode = TCPMODE_INVALID;
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
	m_clientSocket = m_tcpClients.front()->Connect( split[0], static_cast<uint16_t>( atoi( split[1].c_str() ) ), SocketMode::NONBLOCKING );

	m_mode = TCPMODE_CLIENT;

}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::client_disconnect( EventArgs* args )
{
	UNUSED( args );

	g_theConsole->PrintString( Rgba8::GREEN, "Disconnecting from %s...", m_clientSocket.GetAddress().c_str() );
	SendDisconnectMessage();
	m_clientSocket.ShutDown();
	m_clientSocket.Close();

	m_mode = TCPMODE_INVALID;

}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::open_udp_port( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );
	int sendToPort = args->GetValue( "sendToPort", 48001 );

	g_theConsole->PrintString( Rgba8::GREEN, "Opening UDP Port on %i and sending to on %i...", bindPort, sendToPort );

	OpenUDPPort( bindPort, sendToPort );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::send_udp_message( EventArgs* args )
{
	std::string message = args->GetValue( "message", "" );

	g_theConsole->PrintString( Rgba8::GREEN, "Sending Message..." );

	std::string hostData = m_UDPSocket->GetHostData();
	uint16_t sendToPort = static_cast<uint16_t>( m_UDPSocket->GetSendToPort() );

	SendUDPMessage( hostData, sendToPort, 0, 0, message );
}

//---------------------------------------------------------------------------------------------------------
void NetworkSystem::close_udp_port( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );

	g_theConsole->PrintString( Rgba8::GREEN, "Closing port on %i...", bindPort );

	CloseUDPPort( bindPort );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::UDPReceiveMessagesJob( UDPSocket* socket )
{
	while( !m_isUDPSocketQuitting )
	{
		int length = 0;

		length = socket->Receive();

		if( length > 0 )
		{
			UDPMessageHeader const* messageHeader = nullptr;

			auto& buffer = socket->ReceiveBuffer();
			messageHeader = reinterpret_cast<UDPMessageHeader const*>(&buffer[0]);

			UDPMessage messageToReceive;
			messageToReceive.m_header = *messageHeader;
			messageToReceive.m_message = &buffer[sizeof(UDPMessageHeader)];
			m_UDPMessagesToReceive.Push( messageToReceive );
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::microseconds(10) );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::UDPSendMessagesJob( UDPSocket* socket )
{
	while( !m_isUDPSocketQuitting )
	{
		UDPMessage messageToSend;
		if( m_UDPMessagesToSend.Pop( messageToSend ) )
		{
			auto& buffer = socket->SendBuffer();
			*reinterpret_cast<UDPMessageHeader*>(&buffer[0]) = messageToSend.m_header;

			memcpy(&(socket->SendBuffer()[sizeof(UDPMessageHeader)]), messageToSend.m_message.c_str(), messageToSend.m_header.m_size );

			socket->SendBuffer()[sizeof(UDPMessageHeader) + messageToSend.m_header.m_size] = NULL;

			socket->Send(sizeof(UDPMessageHeader) + messageToSend.m_header.m_size + 1);
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::microseconds(10) );
		}
	}
}
