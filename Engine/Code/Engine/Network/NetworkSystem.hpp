#pragma once
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Core/EventSystem.hpp"
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

	void CreateTCPServer( SocketMode mode );
	void CreateTCPClient();

	void SendDisconnectMessage();
	
public:
	void start_tcp_server( EventArgs* args );
	void stop_tcp_server( EventArgs* args );
	void send_message( EventArgs* args );
	void client_connect( EventArgs* args );
	void client_disconnect( EventArgs* args );

private:
	TCPMode m_mode = TCPMODE_INVALID;
	TCPSocket m_clientSocket;
	std::vector<TCPServer*> m_tcpServers;
	std::vector<TCPClient*> m_tcpClients;
};