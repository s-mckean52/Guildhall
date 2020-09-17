#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Network/TCPServer.hpp"
#include "Engine/Network/TCPClient.hpp"
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
	g_theEventSystem->SubscribeEventCallbackMethod( "startTCPServer", this, &NetworkSystem::start_tcp_server );
	g_theEventSystem->SubscribeEventCallbackMethod( "stopTCPServer", this, &NetworkSystem::stop_tcp_server );
	g_theEventSystem->SubscribeEventCallbackMethod( "sendMessage", this, &NetworkSystem::send_message );
	g_theEventSystem->SubscribeEventCallbackMethod( "connect", this, &NetworkSystem::client_connect );
	g_theEventSystem->SubscribeEventCallbackMethod( "disconnect", this, &NetworkSystem::client_disconnect );
}


//---------------------------------------------------------------------------------------------------------
void NetworkSystem::BeginFrame()
{
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
			MessageHeader* messageHeader = reinterpret_cast<MessageHeader*>( &data[0] );
			ServerListeningMessage serverListenMessage;
			serverListenMessage.m_header = *messageHeader;
			serverListenMessage.m_gameName = std::string( &data[4] );

			if( messageHeader->m_id == 3 )
			{
				g_theConsole->PrintString( Rgba8::GREEN, "Client %s is shutting down...", m_clientSocket.GetAddress().c_str() );
				m_clientSocket.ShutDown();
				m_clientSocket.Close();
			}
			else if( buf.GetLength() > 0 )
			{
				g_theConsole->PrintString( Rgba8::WHITE, "Client: "+ serverListenMessage.m_gameName );
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
			MessageHeader* messageHeader = reinterpret_cast<MessageHeader*>( &data[0] );
			ServerListeningMessage serverListenMessage;
			serverListenMessage.m_header = *messageHeader;
			serverListenMessage.m_gameName = std::string( &data[4] );


			if( messageHeader->m_id == 3 )
			{
				g_theConsole->PrintString( Rgba8::GREEN, "Host Server is shutting down..." );
				m_clientSocket.ShutDown();
				m_clientSocket.Close();
			}
			else if( buf.GetLength() > 0 )
			{
				g_theConsole->PrintString( Rgba8::WHITE, "Server: " + serverListenMessage.m_gameName );
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

	MessageHeader messageHeader;
	messageHeader.m_id = 3;
	messageHeader.m_size = 4;
	m_clientSocket.Send( (const char*)&messageHeader, 4 );
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

	MessageHeader* headerPtr = reinterpret_cast<MessageHeader*>( &buffer[0] );
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
