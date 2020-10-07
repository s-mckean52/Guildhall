#pragma once
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/SynchronizedNonBlockingQueue.hpp"
#include <ws2tcpip.h>
#include <winsock2.h>
#include <string>
#include <thread>
#include <vector>

enum TCPMode
{
	TCPMODE_INVALID,
	TCPMODE_SERVER,
	TCPMODE_CLIENT,
};

//---------------------------------------------------------------------------------------------------------
struct TCPMessageHeader
{
	uint16_t m_id;
	uint16_t m_size;
};


//---------------------------------------------------------------------------------------------------------
struct TCPMessage
{
	TCPMessageHeader	m_header;
	std::string			m_message;
};


//---------------------------------------------------------------------------------------------------------
struct UDPMessageHeader
{
	uint16_t m_id;
	uint16_t m_size;
	uint16_t m_seqNo;
};


//---------------------------------------------------------------------------------------------------------
struct UDPMessage
{
	UDPMessageHeader	m_header;
	std::string			m_message;
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
	void CreateTCPServer( SocketMode mode );
	void CreateTCPClient();
	void SendDisconnectMessage();

	//UDP
	void OpenUDPPort( int bindPort, int sendToPort );
	void CloseUDPPort( int bindPort );
	void SendUDPMessage( uint16_t id, uint16_t sequenceNum, std::string const& message );
	void UDPReadMessages();
	
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
	void UDPReceiveMessagesJob( UDPSocket* socket );
	void UDPSendMessagesJob( UDPSocket* socket );

private:
	TCPMode m_mode = TCPMODE_INVALID;
	TCPSocket m_clientSocket;
	std::vector<TCPServer*> m_tcpServers;
	std::vector<TCPClient*> m_tcpClients;

	bool m_isUDPSocketQuitting		= true;
	UDPSocket* m_UDPSocket			= nullptr;
	std::thread m_UDPReadThread;
	std::thread m_UDPSendThread;
	SynchronizedNonBlockingQueue<UDPMessage> m_UDPMessagesToReceive;
	SynchronizedNonBlockingQueue<UDPMessage> m_UDPMessagesToSend;
};