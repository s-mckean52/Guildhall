#pragma once
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/SynchronizedNonBlockingQueue.hpp"
#include "Engine/Network/NetworkMessages.hpp"
#include <ws2tcpip.h>
#include <winsock2.h>
#include <string>
#include <vector>

enum TCPMode
{
	TCPMODE_INVALID,
	TCPMODE_SERVER,
	TCPMODE_CLIENT,
};


class TCPServer;
class TCPClient;
class UDPSocket;

//---------------------------------------------------------------------------------------------------------
class NetworkSystem
{
public:
	NetworkSystem();
	~NetworkSystem();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	//TCP
	void	CreateTCPServer( SocketMode mode );
	void	CreateTCPClient();
	void	DisconnectTCPClient();
	void	CloseTCPServer();
	std::string		GetTCPSocketAddress();
	void	ConnectTCPClient( std::string const& ipAddress, uint16_t portNum, SocketMode socketMode = SocketMode::NONBLOCKING );
	void	SendTCPMessage( TCPMessage tcpMessageToSend );
	void	SendDisconnectMessage();
	void	AppendTCPMessage( TCPMessage const& tcpMessage );
	bool	GetTCPMessage( TCPMessage& out_message );

	//UDP
	void		SendReliableMessages();
	UDPSocket*	OpenUDPPort( std::string const& ipAddress, int bindPort, int sendToPort );
	void		CloseUDPPort( int bindPort );
	void		SendUDPMessage( UDPSocket* socket, UDPMessage const& message );
	void		UDPReadMessages( UDPSocket* socket );
	void		GetUDPMessages( UDPSocket* socket, std::deque<UDPMessage>& out_messages );
	
public:
	void start_tcp_server( EventArgs* args );
	void stop_tcp_server( EventArgs* args );
	void send_message( EventArgs* args );
	void client_connect( EventArgs* args );
	void client_disconnect( EventArgs* args );

	void open_udp_port( EventArgs* args );
	void send_udp_message( EventArgs* args );
	void close_udp_port( EventArgs* args );

private:
	TCPMode m_mode = TCPMODE_INVALID;
	TCPSocket m_clientSocket;
	std::deque<TCPMessage> m_tcpMessages;
	std::deque<UDPMessage> m_udpMessages;
	TCPServer* m_tcpServer;
	TCPClient* m_tcpClient;
	//std::vector<TCPServer*> m_tcpServers;
	//std::vector<TCPClient*> m_tcpClients;

	std::vector<UDPSocket*> m_UDPSockets;
};